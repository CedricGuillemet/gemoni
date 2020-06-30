#include "imgui.h"
#include "GraphEditor.h"
#include "Style.h"
#include <algorithm>

bool mbShowNodes = true;

void UIInit()
{
    SetStyle();
    InitFonts();
}

struct GEDelegate : public GraphEditorDelegate
{
    //NodeIndex mSelectedNodeIndex{ InvalidNodeIndex };
    GEDelegate()
    {
        mNodes.push_back({"My Node", ImRect(ImVec2(0.f,0.f), ImVec2(200.f, 200.f)), 0xFFAAAAAA, 0xFF555555});
        mNodes.push_back({ "My Node", ImRect(ImVec2(300.f,0.f), ImVec2(300 + 200.f, 200.f)), 0xFFAAAAAA, 0xFF555555 });
        mNodes.push_back({ "My Node", ImRect(ImVec2(600.f,0.f), ImVec2(600 + 200.f, 200.f)), 0xFFAAAAAA, 0xFF555555 });
        mNodes.push_back({ "My Node", ImRect(ImVec2(900.f,0.f), ImVec2(900 + 200.f, 200.f)), 0xFFAAAAAA, 0xFF555555 });
        mNodes.push_back({ "My Node", ImRect(ImVec2(1200.f,0.f), ImVec2(1200 + 200.f, 200.f)), 0xFFAAAAAA, 0xFF555555 });
    }
    // getters
    virtual ImVec2 GetEvaluationSize(NodeIndex nodeIndex) const { return ImVec2(150, 150); }
    virtual float NodeProgress(NodeIndex nodeIndex) const { return 0.5f; }

    virtual bool RecurseIsLinked(NodeIndex from, NodeIndex to) const { return false; }

    virtual void DrawNodeImage(ImDrawList* drawList, const ImRect& rc, const ImVec2 marge, NodeIndex nodeIndex) {}
    virtual void ContextMenu(ImVec2 rightclickPos, ImVec2 worldMousePos, int nodeHovered) {}

    virtual void BeginTransaction(bool undoable) {}
    virtual void EndTransaction() {}

    virtual void MoveNodes(std::vector<NodeIndex>& nodes, const ImVec2 delta)
    {
        for (auto nodeIndex : nodes)
        {
            mNodes[nodeIndex].mRect.Min += delta;
            mNodes[nodeIndex].mRect.Max += delta;
        }
    }
    
    virtual void CopyNodes(std::vector<NodeIndex>& nodes)
    {
        for (auto nodeIndex : nodes)
        {
            mClipboard.push_back(mNodes[nodeIndex]);
        }
    }
    
    virtual void DeleteNodes(std::vector<NodeIndex>& nodes)
    {
        std::vector<NodeIndex> sortedIndices = nodes;
        std::sort(sortedIndices.begin(), sortedIndices.end());
        std::reverse(sortedIndices.begin(), sortedIndices.end());
        for (auto nodeIndex : sortedIndices)
        {
            mNodes.erase(mNodes.begin() + nodeIndex);
        }
    }

    virtual std::vector<NodeIndex> PasteNodes(const ImVec2 offset)
    {
        std::vector<NodeIndex> pastedNodeIndex;
        for (auto& node : mClipboard)
        {
            pastedNodeIndex.push_back(NodeIndex(mNodes.size()));
            mNodes.push_back(node);
            mNodes.back().mRect.Min += offset;
            mNodes.back().mRect.Max += offset;
        }
        return pastedNodeIndex;
    }


    virtual void AddLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex) {}
    virtual void DelLink(size_t linkIndex) {}

    virtual const std::vector<GraphEditorDelegate::Node>& GetNodes() const
    {
        return mNodes;
    }

    virtual const std::vector<GraphEditorDelegate::Link>& GetLinks() const
    {
        return mLinks;
    }

    std::vector<GraphEditorDelegate::Node> mNodes;
    std::vector<GraphEditorDelegate::Link> mLinks;

    std::vector<GraphEditorDelegate::Node> mClipboard;
};
void ShowNodeGraph()
{
    static GEDelegate gedelegate;
    GraphEditor(&gedelegate, true/*mSelectedMaterial != -1*/);
}

void UIMain()
{
    ImGuiIO& io = ImGui::GetIO();
    /*mBuilder = builder;
    if (!capturing)
    {
        ShowTitleBar(builder);
    }
    */
    //ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());//:GetCursorPos());
    //ImGui::SetNextWindowSize(ImVec2(200, 200));//io.DisplaySize/* - deltaHeight*/);
    ImVec2 nodesWindowPos;

    // be sure that everything is in sync before rendering them
    //mNodeGraphControler->ApplyDirtyList();

    // MAGIC numbers!
    const ImVec2 windowPos = ImGui::GetCursorScreenPos() - ImVec2(68,86);
    const ImVec2 canvasSize = io.DisplaySize - ImVec2(2,2);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(canvasSize);
    ImGui::Begin("MyDockspace"
    , 0, ImGuiWindowFlags_NoDocking| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus);
    /*if (ImGui::Begin("Imogen",
        0,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus))
        */
    {
        static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);


        // view extraction, just aftper applydirtylist and before node editing in case a node 
        // has been deleted and the extracted views are not aware of
        //ShowExtractedViews();

        //if (mbShowNodes)
        {
            if (ImGui::Begin("Nodes", &mbShowNodes))
            {
                ShowNodeGraph();
            }
            //nodesWindowPos = ImGui::GetWindowPos();
            //interfacesRect["Nodes"] = ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
            ImGui::End();
        }
        /*
        if (mbShowLibrary)
        {
            if (ImGui::Begin("Library", &mbShowLibrary))
            {
                LibraryEdit(library);
            }
            interfacesRect["Library"] = ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
            ImGui::End();
        }

        if (mbShowParameters)
        {
            if (ImGui::Begin("Parameters", &mbShowParameters))
            {
                mNodeGraphControler->NodeEdit();
            }
            interfacesRect["Parameters"] =
                ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
            ImGui::End();
        }

        if (mbShowLog)
        {
            if (ImGui::Begin("Logs", &mbShowLog))
            {
                ImguiAppLog::Log->DrawEmbedded();
            }
            interfacesRect["Logs"] = ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
            ImGui::End();
        }
        */
        //
    }
    ImGui::End();
}