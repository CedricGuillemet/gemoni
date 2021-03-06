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
#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"

typedef unsigned int NodeIndex;
typedef unsigned int SlotIndex;

struct GraphEditorDelegate
{
    // getters
    virtual ImVec2 GetEvaluationSize(NodeIndex nodeIndex) const = 0;
    virtual float NodeProgress(NodeIndex nodeIndex) const = 0;
    virtual bool RecurseIsLinked(NodeIndex from, NodeIndex to) const = 0;

    virtual void DrawNodeImage(ImDrawList* drawList, const ImRect& rc, const ImVec2 marge, NodeIndex nodeIndex) = 0;
    virtual void ContextMenu(ImVec2 rightclickPos, ImVec2 worldMousePos, int nodeHovered) = 0;

    // operations
    virtual void BeginTransaction(bool undoable) = 0;
    virtual void EndTransaction() = 0;

    virtual void MoveNodes(std::vector<NodeIndex>& nodes, const ImVec2 delta) = 0;
    virtual void CopyNodes(std::vector<NodeIndex>& nodes) = 0;
    virtual void DeleteNodes(std::vector<NodeIndex>& nodes) = 0;
    virtual std::vector<NodeIndex> PasteNodes(const ImVec2 offset) = 0;
    
    virtual void AddLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex) = 0;
    virtual void DelLink(size_t linkIndex) = 0;

    struct Node
    {
        const char *mName;
        ImRect mRect;
        uint32_t mHeaderColor;
        uint32_t mBackgroundColor;
        std::vector<const char*> mInputs;
        std::vector<const char*> mOutputs;
    };

    struct Link
    {
        int mInputNodeIndex, mInputSlotIndex, mOutputNodeIndex, mOutputSlotIndex;
    };

    // node/links/rugs retrieval
    virtual const std::vector<Node>& GetNodes() const = 0;
    virtual const std::vector<Link>& GetLinks() const = 0;
};

void GraphEditor(GraphEditorDelegate* delegate, bool enabled);
void GraphEditorClear();

void GraphEditorUpdateScrolling(GraphEditorDelegate* delegate);

