// https://github.com/CedricGuillemet/Imogen
//
// The MIT License(MIT)
//
// Copyright(c) 2019 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <vector>
#include <float.h>
#include <array>
#include <set>
#include <map>
#include <algorithm>
#include "GraphEditor.h"

static inline float Distance(ImVec2& a, ImVec2& b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

static inline float sign(float v)
{
    return (v >= 0.f) ? 1.f : -1.f;
}

ImVec2 GetInputSlotPos(const GraphEditorDelegate::Node& node, int slot_no, float factor)
{
    ImVec2 Size = node.mRect.GetSize() * factor;
    size_t InputsCount = node.mInputs.size();
    return ImVec2(node.mRect.Min.x * factor,
                  node.mRect.Min.y * factor + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1) + 8.f);
}

ImVec2 GetOutputSlotPos(const GraphEditorDelegate::Node& node, int slot_no, float factor)
{
    ImVec2 Size = node.mRect.GetSize() * factor;
    size_t OutputsCount = node.mOutputs.size();
    return ImVec2(node.mRect.Min.x * factor + Size.x,
                  node.mRect.Min.y * factor + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1) + 8.f);
}

ImRect GetNodeRect(const GraphEditorDelegate::Node& node, float factor)
{
    ImVec2 Size = node.mRect.GetSize() * factor;
    return ImRect(node.mRect.Min * factor, node.mRect.Min * factor + Size);
}

static const float NODE_SLOT_RADIUS = 8.0f;
static const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

static ImVec2 editingNodeSource;
bool editingInput = false;
static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
float factor = 1.0f;
float factorTarget = 1.0f;
ImVec2 captureOffset;
static bool inTransaction = false;
static std::set<int> selectedNodes;
ImVec2 movingNodesOffset;
enum NodeOperation
{
    NO_None,
    NO_EditingLink,
    NO_QuadSelecting,
    NO_MovingNodes,
    NO_EditInput,
    NO_PanView,
};
NodeOperation nodeOperation = NO_None;

void GraphEditorClear()
{
    nodeOperation = NO_None;
    factor = 1.0f;
    factorTarget = 1.0f;
    selectedNodes.clear();
}

std::vector<NodeIndex> GetSelectedNodes()
{
    std::vector<NodeIndex> res;
    for (auto selectedNodeIndex : selectedNodes)
    {
        res.push_back(selectedNodeIndex);
    }
    return res;
}

void UnSelectNodes()
{
    selectedNodes.clear();
}

bool IsNodeSelected(int nodeIndex)
{
    return std::find(selectedNodes.begin(), selectedNodes.end(), nodeIndex) != selectedNodes.end();
}

void SelectNode(int nodeIndex, bool selected)
{
    auto iter = std::find(selectedNodes.begin(), selectedNodes.end(), nodeIndex);
    if (iter != selectedNodes.end())
    {
        if (!selected)
        {
            selectedNodes.erase(iter);
        }

    }
    else
    {
        if (selected)
        {
            selectedNodes.insert(nodeIndex);
        }
    }
}

void HandleZoomScroll(ImRect regionRect)
{
    ImGuiIO& io = ImGui::GetIO();

    if (regionRect.Contains(io.MousePos))
    {
        if (io.MouseWheel < -FLT_EPSILON)
            factorTarget *= 0.9f;

        if (io.MouseWheel > FLT_EPSILON)
            factorTarget *= 1.1f;
    }

    ImVec2 mouseWPosPre = (io.MousePos - ImGui::GetCursorScreenPos()) / factor;
    factorTarget = ImClamp(factorTarget, 0.2f, 3.f);
    factor = ImLerp(factor, factorTarget, 0.15f);
    ImVec2 mouseWPosPost = (io.MousePos - ImGui::GetCursorScreenPos()) / factor;
    if (ImGui::IsMousePosValid())
    {
        scrolling += mouseWPosPost - mouseWPosPre;
    }
}

void GraphEditorUpdateScrolling(GraphEditorDelegate *delegate)
{
    const auto& nodes = delegate->GetNodes();

    if (nodes.empty())
        return;

    if (!nodes.empty())
    {
        scrolling = nodes[0].mRect.Min;
    }
    else
    {
        scrolling = ImVec2(0, 0);
    }
    for (auto& node : nodes)
    {
        scrolling.x = std::min(scrolling.x, node.mRect.Min.x);
        scrolling.y = std::min(scrolling.y, node.mRect.Min.y);
    }

    scrolling = ImVec2(40, 40) - scrolling;
}

static void DisplayLinks(GraphEditorDelegate* delegate,
                         ImDrawList* drawList,
                         const ImVec2 offset,
                         const float factor,
                         const ImRect regionRect,
                         int hoveredNode)
{
    const auto& links = delegate->GetLinks();
    const auto& nodes = delegate->GetNodes();
    for (int link_idx = 0; link_idx < links.size(); link_idx++)
    {
        const auto* link = &links[link_idx];
        const auto* node_inp = &nodes[link->mInputNodeIndex];
        const auto* node_out = &nodes[link->mOutputNodeIndex];
        ImVec2 p1 = offset + GetOutputSlotPos(*node_inp, link->mInputSlotIndex, factor);
        ImVec2 p2 = offset + GetInputSlotPos(*node_out, link->mOutputSlotIndex, factor);

        // con. view clipping
        if ((p1.y < 0.f && p2.y < 0.f) || (p1.y > regionRect.Max.y && p2.y > regionRect.Max.y) ||
            (p1.x < 0.f && p2.x < 0.f) || (p1.x > regionRect.Max.x && p2.x > regionRect.Max.x))
            continue;

        bool highlightCons = hoveredNode == link->mInputNodeIndex || hoveredNode == link->mOutputNodeIndex;
        uint32_t col = node_inp->mHeaderColor | (highlightCons ? 0xF0F0F0 : 0);
        ;
        // curves
        // drawList->AddBezierCurve(p1, p1 + ImVec2(+50, 0) * factor, p2 + ImVec2(-50, 0) * factor, p2, 0xFF000000, 4.f
        // * factor); drawList->AddBezierCurve(p1, p1 + ImVec2(+50, 0) * factor, p2 + ImVec2(-50, 0) * factor, p2,
        // col, 3.0f * factor);

        // -/-
        /*
        ImVec2 p10 = p1 + ImVec2(20.f * factor, 0.f);
        ImVec2 p20 = p2 - ImVec2(20.f * factor, 0.f);

        ImVec2 dif = p20 - p10;
        ImVec2 p1a, p1b;
        if (fabsf(dif.x) > fabsf(dif.y))
        {
            p1a = p10 + ImVec2(fabsf(fabsf(dif.x) - fabsf(dif.y)) * 0.5 * sign(dif.x), 0.f);
            p1b = p1a + ImVec2(fabsf(dif.y) * sign(dif.x) , dif.y);
        }
        else
        {
            p1a = p10 + ImVec2(0.f, fabsf(fabsf(dif.y) - fabsf(dif.x)) * 0.5 * sign(dif.y));
            p1b = p1a + ImVec2(dif.x, fabsf(dif.x) * sign(dif.y));
        }
        drawList->AddLine(p1,  p10, col, 3.f * factor);
        drawList->AddLine(p10, p1a, col, 3.f * factor);
        drawList->AddLine(p1a, p1b, col, 3.f * factor);
        drawList->AddLine(p1b, p20, col, 3.f * factor);
        drawList->AddLine(p20,  p2, col, 3.f * factor);
        */
        std::array<ImVec2, 6> pts;
        int ptCount = 0;
        ImVec2 dif = p2 - p1;

        ImVec2 p1a, p1b;
        const float limitx = 12.f * factor;
        if (dif.x < limitx)
        {
            ImVec2 p10 = p1 + ImVec2(limitx, 0.f);
            ImVec2 p20 = p2 - ImVec2(limitx, 0.f);

            dif = p20 - p10;
            p1a = p10 + ImVec2(0.f, dif.y * 0.5f);
            p1b = p1a + ImVec2(dif.x, 0.f);

            pts = {p1, p10, p1a, p1b, p20, p2};
            ptCount = 6;
        }
        else
        {
            if (fabsf(dif.y) < 1.f)
            {
                pts = {p1, (p1 + p2) * 0.5f, p2};
                ptCount = 3;
            }
            else
            {
                if (fabsf(dif.y) < 10.f)
                {
                    if (fabsf(dif.x) > fabsf(dif.y))
                    {
                        p1a = p1 + ImVec2(fabsf(fabsf(dif.x) - fabsf(dif.y)) * 0.5f * sign(dif.x), 0.f);
                        p1b = p1a + ImVec2(fabsf(dif.y) * sign(dif.x), dif.y);
                    }
                    else
                    {
                        p1a = p1 + ImVec2(0.f, fabsf(fabsf(dif.y) - fabsf(dif.x)) * 0.5f * sign(dif.y));
                        p1b = p1a + ImVec2(dif.x, fabsf(dif.x) * sign(dif.y));
                    }
                }
                else
                {
                    if (fabsf(dif.x) > fabsf(dif.y))
                    {
                        float d = fabsf(dif.y) * sign(dif.x) * 0.5f;
                        p1a = p1 + ImVec2(d, dif.y * 0.5f);
                        p1b = p1a + ImVec2(fabsf(fabsf(dif.x) - fabsf(d) * 2.f) * sign(dif.x), 0.f);
                    }
                    else
                    {
                        float d = fabsf(dif.x) * sign(dif.y) * 0.5f;
                        p1a = p1 + ImVec2(dif.x * 0.5f, d);
                        p1b = p1a + ImVec2(0.f, fabsf(fabsf(dif.y) - fabsf(d) * 2.f) * sign(dif.y));
                    }
                }
                pts = {p1, p1a, p1b, p2};
                ptCount = 4;
            }
        }
        float highLightFactor = factor * (highlightCons ? 2.0f : 1.f);
        for (int pass = 0; pass < 2; pass++)
        {
            drawList->AddPolyline(
                pts.data(), ptCount, pass ? col : 0xFF000000, false, (pass ? 5.f : 7.5f) * highLightFactor);
        }
    }
}

static void HandleQuadSelection(
    GraphEditorDelegate* delegate, ImDrawList* drawList, const ImVec2 offset, const float factor, ImRect contentRect)
{
    ImGuiIO& io = ImGui::GetIO();
    static ImVec2 quadSelectPos;
    auto& nodes = delegate->GetNodes();

    if (nodeOperation == NO_QuadSelecting && ImGui::IsWindowFocused())
    {
        const ImVec2 bmin = ImMin(quadSelectPos, io.MousePos);
        const ImVec2 bmax = ImMax(quadSelectPos, io.MousePos);
        drawList->AddRectFilled(bmin, bmax, 0x40FF2020, 1.f);
        drawList->AddRect(bmin, bmax, 0xFFFF2020, 1.f);
        if (!io.MouseDown[0])
        {
            if (!io.KeyCtrl && !io.KeyShift)
            {
                UnSelectNodes();
            }

            nodeOperation = NO_None;
            ImRect selectionRect(bmin, bmax);
            for (int nodeIndex = 0; nodeIndex < nodes.size(); nodeIndex++)
            {
                const auto* node = &nodes[nodeIndex];
                ImVec2 node_rect_min = offset + node->mRect.Min * factor;
                ImVec2 node_rect_max = node_rect_min + node->mRect.GetSize() * factor;
                if (selectionRect.Overlaps(ImRect(node_rect_min, node_rect_max)))
                {
                    if (io.KeyCtrl)
                    {
                        SelectNode(nodeIndex, false);
                    }
                    else
                    {
                        SelectNode(nodeIndex, true);
                    }
                }
                else
                {
                    if (!io.KeyShift)
                    {
                        SelectNode(nodeIndex, false);
                    }
                }
            }
        }
    }
    else if (nodeOperation == NO_None && io.MouseDown[0] && ImGui::IsWindowFocused() &&
             contentRect.Contains(io.MousePos) )
    {
        nodeOperation = NO_QuadSelecting;
        quadSelectPos = io.MousePos;
    }
}

static int gvalue = 0;
void HandleCopyCutPasteDelete(GraphEditorDelegate* delegate)
{
    ImGuiIO& io = ImGui::GetIO();
    static bool armed = true;

    bool keyC = ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_C], false);
    bool keyV = ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_V], false);
    bool keyX = ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_X], false);
    bool keyDel = ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Delete], false);

    if (!keyC && !keyV && !keyDel && !keyX)
    {
        armed = true;
    }

    if (!armed)
    {
        return;
    }

    if (keyC || keyV || keyDel || keyX)
    {
        armed = false;
    }

    if (io.KeyCtrl && keyC)
    {
        auto nodes = GetSelectedNodes();
        delegate->CopyNodes(nodes);
        
    }
    if (io.KeyCtrl && keyX)
    {
        auto nodes = GetSelectedNodes();
        delegate->CopyNodes(nodes);
        delegate->DeleteNodes(nodes);
        UnSelectNodes();
    }
    if (io.KeyCtrl && keyV)
    {
        auto pastedNodes = delegate->PasteNodes(ImVec2(40.f, 40.f));
        UnSelectNodes();
        for (auto nodeIndex : pastedNodes)
        {
            SelectNode(nodeIndex, true);
        }
    }
    if (keyDel)
    {
        auto nodes = GetSelectedNodes();
        delegate->DeleteNodes(nodes);
        UnSelectNodes();
    }
}

bool HandleConnections(ImDrawList* drawList,
                       int nodeIndex,
                       const ImVec2 offset,
                       const float factor,
                       GraphEditorDelegate* delegate,
                       bool bDrawOnly)
{
    static int editingNodeIndex;
    static int editingSlotIndex;
    const auto& links = delegate->GetLinks();
    const auto& nodes = delegate->GetNodes();

    
    HandleCopyCutPasteDelete(delegate);
    
    ImGuiIO& io = ImGui::GetIO();
    const GraphEditorDelegate::Node* node = &nodes[nodeIndex];

    size_t InputsCount = node->mInputs.size();
    size_t OutputsCount = node->mOutputs.size();

    // draw/use inputs/outputs
    bool hoverSlot = false;
    for (int i = 0; i < 2; i++)
    {
        float closestDistance = FLT_MAX;
        int closestConn = -1;
        ImVec2 closestTextPos;
        ImVec2 closestPos;
        const size_t slotCount[2] = {InputsCount, OutputsCount};
        
        for (int slot_idx = 0; slot_idx < slotCount[i]; slot_idx++)
        {
            const char* con = i ? node->mOutputs[slot_idx] : node->mInputs[slot_idx];
            const char* conText = con ? con : "";

            ImVec2 p =
                offset + (i ? GetOutputSlotPos(*node, slot_idx, factor) : GetInputSlotPos(*node, slot_idx, factor));
            float distance = Distance(p, io.MousePos);
            bool overCon = (nodeOperation == NO_None || nodeOperation == NO_EditingLink) &&
                           (distance < NODE_SLOT_RADIUS * 2.f) && (distance < closestDistance);

            
            ImVec2 textSize;
            textSize = ImGui::CalcTextSize(conText);
            ImVec2 textPos =
                p + ImVec2(-NODE_SLOT_RADIUS * (i ? -1.f : 1.f) * (overCon ? 3.f : 2.f) - (i ? 0 : textSize.x),
                           -textSize.y / 2);

            ImRect nodeRect = GetNodeRect(*node, factor);
            if (overCon || (nodeRect.Contains(io.MousePos - offset) && closestConn == -1 &&
                            (editingInput == (i != 0)) && nodeOperation == NO_EditingLink))
            {
                closestDistance = distance;
                closestConn = slot_idx;
                closestTextPos = textPos;
                closestPos = p;
            }

            drawList->AddCircleFilled(p, NODE_SLOT_RADIUS * 1.2f, IM_COL32(0, 0, 0, 200));
            drawList->AddCircleFilled(p, NODE_SLOT_RADIUS * 0.75f * 1.2f, IM_COL32(160, 160, 160, 200));
            drawList->AddText(io.FontDefault, 14, textPos + ImVec2(2, 2), IM_COL32(0, 0, 0, 255), conText);
            drawList->AddText(io.FontDefault, 14, textPos, IM_COL32(150, 150, 150, 255), conText);
        }

        if (closestConn != -1)
        {
            const char* con = i ? node->mOutputs[closestConn] : node->mInputs[closestConn];
            const char* conText = con ? con : "";
            hoverSlot = true;
            drawList->AddCircleFilled(closestPos, NODE_SLOT_RADIUS * 2.f, IM_COL32(0, 0, 0, 200));
            drawList->AddCircleFilled(closestPos, NODE_SLOT_RADIUS * 1.5f, IM_COL32(200, 200, 200, 200));
            drawList->AddText(io.FontDefault, 16, closestTextPos + ImVec2(1, 1), IM_COL32(0, 0, 0, 255), conText);
            drawList->AddText(io.FontDefault, 16, closestTextPos, IM_COL32(250, 250, 250, 255), conText);
            bool inputToOutput = (!editingInput && !i) || (editingInput && i);
            if (nodeOperation == NO_EditingLink && !io.MouseDown[0] && !bDrawOnly)
            {
                if (inputToOutput)
                {
                    // check loopback
                    GraphEditorDelegate::Link nl;
                    if (editingInput)
                        nl = GraphEditorDelegate::Link{nodeIndex, closestConn, editingNodeIndex, editingSlotIndex};
                    else
                        nl = GraphEditorDelegate::Link{editingNodeIndex, editingSlotIndex, nodeIndex, closestConn};

                    if (delegate->RecurseIsLinked(nl.mOutputNodeIndex, nl.mInputNodeIndex))
                    {
                        break;
                    }
                    bool alreadyExisting = false;
                    for (int linkIndex = 0; linkIndex < links.size(); linkIndex++)
                    {
                        if (!memcmp(&links[linkIndex], &nl, sizeof(GraphEditorDelegate::Link)))
                        {
                            alreadyExisting = true;
                            break;
                        }
                    }

                    // check already connected output
                    for (int linkIndex = 0; linkIndex < links.size(); linkIndex++)
                    {
                        auto& link = links[linkIndex];
                        if (link.mOutputNodeIndex == nl.mOutputNodeIndex && link.mOutputSlotIndex == nl.mOutputSlotIndex)
                        {
                            if (!inTransaction)
                            {
                                delegate->BeginTransaction(true);
                                inTransaction = true;
                            }
                            delegate->DelLink(linkIndex);
                            
                            break;
                        }
                    }

                    if (!alreadyExisting)
                    {
                        if (!inTransaction)
                        {
                            delegate->BeginTransaction(true);
                            inTransaction = true;
                        }
                        delegate->AddLink(nl.mInputNodeIndex, nl.mInputSlotIndex, nl.mOutputNodeIndex, nl.mOutputSlotIndex);
                    }
                }
            }
            // when ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() is uncommented, one can't click the node
            // input/output when mouse is over the node itself.
            if (nodeOperation == NO_None &&
                /*ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&*/ io.MouseClicked[0] && !bDrawOnly)
            {
                nodeOperation = NO_EditingLink;
                editingInput = i == 0;
                editingNodeSource = closestPos;
                editingNodeIndex = nodeIndex;
                editingSlotIndex = closestConn;
                if (editingInput)
                {
                    // remove existing link
                    for (int linkIndex = 0; linkIndex < links.size(); linkIndex++)
                    {
                        auto& link = links[linkIndex];
                        if (link.mOutputNodeIndex == nodeIndex && link.mOutputSlotIndex == closestConn)
                        {
                            if (!inTransaction)
                            {
                                delegate->BeginTransaction(true);
                                inTransaction = true;
                            }
                            delegate->DelLink(linkIndex);
                            break;
                        }
                    }
                }
            }
        }
    }
    if (nodeOperation == NO_EditingLink && inTransaction)
    {
        delegate->EndTransaction();
        inTransaction = false;
    }
    return hoverSlot;
}

static void DrawGrid(ImDrawList* drawList, ImVec2 windowPos, const ImVec2 canvasSize, const float factor)
{
    ImU32 GRID_COLOR = IM_COL32(100, 100, 100, 40);
    float GRID_SZ = 64.0f * factor;
    for (float x = fmodf(scrolling.x * factor, GRID_SZ); x < canvasSize.x; x += GRID_SZ)
        drawList->AddLine(ImVec2(x, 0.0f) + windowPos, ImVec2(x, canvasSize.y) + windowPos, GRID_COLOR);
    for (float y = fmodf(scrolling.y * factor, GRID_SZ); y < canvasSize.y; y += GRID_SZ)
        drawList->AddLine(ImVec2(0.0f, y) + windowPos, ImVec2(canvasSize.x, y) + windowPos, GRID_COLOR);
}

// return true if node is hovered
static bool DrawNode(ImDrawList* drawList,
                     int nodeIndex,
                     const ImVec2 offset,
                     const float factor,
                     GraphEditorDelegate* delegate,
                     bool overInput)
{
    ImGuiIO& io = ImGui::GetIO();
    const auto& nodes = delegate->GetNodes();
    const auto* node = &nodes[nodeIndex];

    ImRect nodeRect = node->mRect;
    auto iter = std::find(selectedNodes.begin(), selectedNodes.end(), nodeIndex);
    if (iter != selectedNodes.end())
    {
        nodeRect.Min += movingNodesOffset;
        nodeRect.Max += movingNodesOffset;
    }

    const ImVec2 node_rect_min = offset + nodeRect.Min * factor;

    const bool old_any_active = ImGui::IsAnyItemActive();
    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
    const ImVec2 nodeSize = nodeRect.GetSize() * factor;

    // test nested IO
    drawList->ChannelsSetCurrent(1); // Background
    const size_t InputsCount = node->mInputs.size();
    const size_t OutputsCount = node->mOutputs.size();
    /*
    for (int i = 0; i < 2; i++)
    {
        const size_t slotCount[2] = {InputsCount, OutputsCount};
        
        for (int slot_idx = 0; slot_idx < slotCount[i]; slot_idx++)
        {
            const char* con = i ? node->mOutputs[slot_idx] : node->mInputs[slot_idx];
            if (!delegate->IsIOPinned(nodeIndex, slot_idx, i == 1))
            {
                continue;
            }
            ImVec2 p =
                offset + (i ? GetOutputSlotPos(*node, slot_idx, factor) : GetInputSlotPos(*node, slot_idx, factor));
            const float arc = 28.f * (float(i) * 0.3f + 1.0f) * (i ? 1.f : -1.f);
            const float ofs = 0.f;

            ImVec2 pts[3] = {p + ImVec2(arc + ofs, 0.f), p + ImVec2(0.f + ofs, -arc), p + ImVec2(0.f + ofs, arc)};
            drawList->AddTriangleFilled(pts[0], pts[1], pts[2], i ? 0xFFAA5030 : 0xFF30AA50);
            drawList->AddTriangle(pts[0], pts[1], pts[2], 0xFF000000, 2.f);
        }
    }
    */
    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("node", nodeSize);
    // must be called right after creating the control we want to be able to move
    bool node_moving_active = ImGui::IsItemActive();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
    ImVec2 node_rect_max = node_rect_min + nodeSize;

    bool nodeHovered = false;
    if (ImGui::IsItemHovered() && nodeOperation == NO_None && !overInput)
    {
        nodeHovered = true;
    }

    if (ImGui::IsWindowFocused())
    {
        if (node_widgets_active || node_moving_active)
        {
            if (!IsNodeSelected(nodeIndex))
            {
                if (!io.KeyShift)
                {
                    for (auto i = 0; i < nodes.size(); i++)
                    {
                        SelectNode(i, false);
                    }
                }
                SelectNode(nodeIndex, true);
            }
        }
    }
    if (node_moving_active && io.MouseDown[0] && nodeHovered)
    {
        if (nodeOperation != NO_MovingNodes)
        {
            nodeOperation = NO_MovingNodes;
        }
    }

    bool currentSelectedNode = IsNodeSelected(nodeIndex);


    ImU32 node_bg_color = node->mBackgroundColor + (nodeHovered?0x191919:0);

    drawList->AddRect(node_rect_min,
                      node_rect_max,
                      currentSelectedNode ? IM_COL32(255, 130, 30, 255) : IM_COL32(100, 100, 100, 0),
                      2.0f,
                      15,
                      currentSelectedNode ? 6.f : 2.f);

    ImVec2 imgPos = node_rect_min + ImVec2(14, 25);
    ImVec2 imgSize = node_rect_max + ImVec2(-5, -5) - imgPos;
    float imgSizeComp = std::min(imgSize.x, imgSize.y);

    drawList->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 2.0f);
    float progress = delegate->NodeProgress(nodeIndex);
    if (progress > FLT_EPSILON && progress < 1.f - FLT_EPSILON)
    {
        ImVec2 progressLineA = node_rect_max - ImVec2(nodeSize.x - 2.f, 3.f);
        ImVec2 progressLineB = progressLineA + ImVec2(nodeSize.x - 4.f, 0.f);
        drawList->AddLine(progressLineA, progressLineB, 0xFF400000, 3.f);
        drawList->AddLine(progressLineA, ImLerp(progressLineA, progressLineB, progress), 0xFFFF0000, 3.f);
    }
    ImVec2 imgPosMax = imgPos + ImVec2(imgSizeComp, imgSizeComp);

    ImVec2 imageSize = delegate->GetEvaluationSize(nodeIndex);
    float imageRatio = 1.f;
    if (imageSize.x > 0.f && imageSize.y > 0.f)
    {
        imageRatio = imageSize.y / imageSize.x;
    }
    ImVec2 quadSize = imgPosMax - imgPos;
    ImVec2 marge(0.f, 0.f);
    if (imageRatio > 1.f)
    {
        marge.x = (quadSize.x - quadSize.y / imageRatio) * 0.5f;
    }
    else
    {
        marge.y = (quadSize.y - quadSize.y * imageRatio) * 0.5f;
    }

    delegate->DrawNodeImage(drawList, ImRect(imgPos, imgPosMax), marge, nodeIndex);

    drawList->AddRectFilled(node_rect_min,
                            ImVec2(node_rect_max.x, node_rect_min.y + 20),
                            node->mHeaderColor,
                            2.0f);
    drawList->PushClipRect(node_rect_min, ImVec2(node_rect_max.x, node_rect_min.y + 20), true);
    drawList->AddText(node_rect_min + ImVec2(2, 2), IM_COL32(0, 0, 0, 255), node->mName);
    drawList->PopClipRect();

    /*
    const ImTextureID bmpInfo = (ImTextureID)(uint64_t)delegate->GetBitmapInfo(nodeIndex).idx;
    if (bmpInfo)
    {
        ImVec2 bmpInfoPos(node_rect_max - ImVec2(26, 12));
        ImVec2 bmpInfoSize(20, 20);
        if (delegate->NodeIsCompute(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos,
                                   bmpInfoPos + ImVec2(bmpInfoSize.x, 0.f),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + ImVec2(0., bmpInfoSize.y));
        }
        else if (delegate->NodeIs2D(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos,
                                   bmpInfoPos + ImVec2(bmpInfoSize.x, 0.f),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + ImVec2(0., bmpInfoSize.y));
        }
        else if (delegate->NodeIsCubemap(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos + ImVec2(0., bmpInfoSize.y),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + ImVec2(bmpInfoSize.x, 0.f),
                                   bmpInfoPos);
        }
    }
    */
    return nodeHovered;
}

void ComputeDelegateSelection(GraphEditorDelegate* delegate)
{
    const auto& nodes = delegate->GetNodes();
    // only one selection allowed for delegate
	/*delegate->mSelectedNodeIndex = {InvalidNodeIndex};
    for (auto& node : nodes)
    {
        if (node.mbSelected)
        {
            if (!delegate->mSelectedNodeIndex.IsValid())
            {
                delegate->mSelectedNodeIndex = int(&node - nodes.data());
            }
            else
            {
                delegate->mSelectedNodeIndex.SetInvalid();
                return;
            }
        }
    }*/
}

void GraphEditor(GraphEditorDelegate* delegate, bool enabled)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);

    const ImVec2 windowPos = ImGui::GetCursorScreenPos();
    const ImVec2 canvasSize = ImGui::GetWindowSize();
    const ImVec2 scrollRegionLocalPos(0, 0);
    const auto& nodes = delegate->GetNodes();

    bool openContextMenu = false;

    static ImVec2 scenePos;

    ImRect regionRect(windowPos, windowPos + canvasSize);

    HandleZoomScroll(regionRect);
    ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling * factor;
    captureOffset = scrollRegionLocalPos + scrolling * factor + ImVec2(10.f, 0.f);

    ImGui::SetCursorPos(windowPos);
    ImGui::BeginGroup();


    ImGuiIO& io = ImGui::GetIO();

    // Create our child canvas
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 200));

    ImGui::SetCursorPos(scrollRegionLocalPos);
    ImGui::BeginChild("scrolling_region",
                      ImVec2(0, 0),
                      true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    ImGui::PushItemWidth(120.0f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Background or Display grid
    //if (!delegate->RenderBackground())
    {
        DrawGrid(drawList, windowPos, canvasSize, factor);
    }

    if (!enabled)
    {
        goto nodeGraphExit;
    }

    static int hoveredNode = -1;
    // Display links
    drawList->ChannelsSplit(3);
    drawList->ChannelsSetCurrent(1); // Background
    DisplayLinks(delegate, drawList, offset, factor, regionRect, hoveredNode);

    // edit node link
    if (nodeOperation == NO_EditingLink)
    {
        ImVec2 p1 = editingNodeSource;
        ImVec2 p2 = io.MousePos;
        drawList->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 3.0f);
    }

    // Display nodes
    drawList->PushClipRect(regionRect.Min, regionRect.Max, true);
    hoveredNode = -1;
    for (int i = 0; i < 1; i++)
    {
        for (int nodeIndex = 0; nodeIndex < nodes.size(); nodeIndex++)
        {
            const auto* node = &nodes[nodeIndex];
            /*if (node->mbSelected != (i != 0))
            {
                continue;
            }*/

            // node view clipping
            ImRect nodeRect = GetNodeRect(*node, factor);
            nodeRect.Min += offset;
            nodeRect.Max += offset;
            if (!regionRect.Overlaps(nodeRect))
            {
                continue;
            }

            ImGui::PushID(nodeIndex);

            // Display node contents first
            // drawList->ChannelsSetCurrent(i+1); // channel 2 = Foreground channel 1 = background

            bool overInput = HandleConnections(drawList, nodeIndex, offset, factor, delegate, false);

            if (DrawNode(drawList, nodeIndex, offset, factor, delegate, overInput))
                hoveredNode = nodeIndex;

            HandleConnections(drawList, nodeIndex, offset, factor, delegate, true);

            ImGui::PopID();
        }
    }
    drawList->PopClipRect();

    if (nodeOperation == NO_MovingNodes)
    {
        if (ImGui::IsMouseDragging(0, 1))
        {
            movingNodesOffset += io.MouseDelta / factor;
        }
    }

    drawList->ChannelsSetCurrent(0);

    // quad selection
    HandleQuadSelection(delegate, drawList, offset, factor, regionRect);

    drawList->ChannelsMerge();

    // releasing mouse button means it's done in any operation
    if (nodeOperation == NO_PanView)
    {
        if (!io.MouseDown[2])
        {
            nodeOperation = NO_None;
        }
    }
    else if (nodeOperation != NO_None && !io.MouseDown[0])
    {
        nodeOperation = NO_None;
        if (fabsf(movingNodesOffset.x) > FLT_EPSILON || fabsf(movingNodesOffset.y) > FLT_EPSILON)
        {
            std::vector<NodeIndex> movedNodes = GetSelectedNodes();
            delegate->MoveNodes(movedNodes, movingNodesOffset);
            movingNodesOffset = ImVec2(0.f, 0.f);
        }
    }

    // Open context menu
    static int contextMenuHoverNode = -1;
    if (nodeOperation == NO_None && regionRect.Contains(io.MousePos) &&
        (ImGui::IsMouseClicked(1) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Tab))))
    {
        openContextMenu = true;
        contextMenuHoverNode = hoveredNode;
    }

    if (openContextMenu)
    {
        scenePos = (ImGui::GetMousePosOnOpeningCurrentPopup() - offset) / factor;
        ImGui::OpenPopup("context_menu");
    }
    ;

    delegate->ContextMenu(scenePos, (io.MousePos - offset) / factor, contextMenuHoverNode);

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && io.MouseClicked[2] && nodeOperation == NO_None)
    {
        nodeOperation = NO_PanView;
    }
    if (nodeOperation == NO_PanView)
    {
        scrolling += io.MouseDelta / factor;
    }

nodeGraphExit:;
    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);

    ComputeDelegateSelection(delegate);

    ImGui::EndGroup();
    ImGui::PopStyleVar(3);
}
