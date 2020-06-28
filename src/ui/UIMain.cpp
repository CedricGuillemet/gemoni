#include "imgui.h"
#include "GraphEditor.h"


bool mbShowNodes = true;


struct GEDelegate : public GraphEditorDelegate
{
    //NodeIndex mSelectedNodeIndex{ InvalidNodeIndex };

    // getters
    virtual ImVec2 GetEvaluationSize(NodeIndex nodeIndex) const { return ImVec2(150, 150); }
    virtual int NodeIsProcesing(NodeIndex nodeIndex) const { return false; }
    virtual float NodeProgress(NodeIndex nodeIndex) const { return 0.f; }
    virtual bool NodeIsCubemap(NodeIndex nodeIndex) const { return false; }
    virtual bool NodeIs2D(NodeIndex nodeIndex) const { return true; }
    //virtual bool NodeIsCompute(NodeIndex nodeIndex) const { return false; }
    //virtual bool IsIOPinned(NodeIndex nodeIndex, size_t io, bool forOutput) const = 0;
    virtual bool RecurseIsLinked(NodeIndex from, NodeIndex to) const { return false; }
    //virtual bgfx::TextureHandle GetBitmapInfo(NodeIndex nodeIndex) const = 0;

    virtual void DrawNodeImage(ImDrawList* drawList, const ImRect& rc, const ImVec2 marge, NodeIndex nodeIndex) {}
    virtual void ContextMenu(ImVec2 rightclickPos, ImVec2 worldMousePos, int nodeHovered) {}

    // operations
    virtual bool InTransaction() { return false; }
    virtual void BeginTransaction(bool undoable) {}
    virtual void EndTransaction() {}

    virtual void MoveSelectedNodes(const ImVec2 delta) {}

    virtual void AddLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex) {}
    virtual void DelLink(size_t linkIndex) {}

    // return false if background must be rendered by node graph
    //virtual bool RenderBackground() = 0;

    struct Node
    {
        const char* mName;
        ImRect mRect;
        uint32_t mHeaderColor;
        uint32_t mBackgroundColor;
        std::vector<const char*> mInputs;
        std::vector<const char*> mOutputs;
        bool mbSelected;
    };

    struct Link
    {
        int mInputNodeIndex, mInputSlotIndex, mOutputNodeIndex, mOutputSlotIndex;
    };

    // node/links/rugs retrieval
    virtual const std::vector<GraphEditorDelegate::Node>& GetNodes() const {
        return nodes;
    }
    virtual const std::vector<GraphEditorDelegate::Link>& GetLinks() const
    {
        return links;
    }

    std::vector<GraphEditorDelegate::Node> nodes;
    std::vector<GraphEditorDelegate::Link> links;
};
void ShowNodeGraph()
{
    GEDelegate gedelegate;
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
    const ImVec2 canvasSize = io.DisplaySize - ImVec2(1,1);
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