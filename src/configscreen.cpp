//------------------------------------------------------------------
//
//this file is part of Proximodo
//Copyright (C) 2004 Antony BOUCHER ( kuruden@users.sourceforge.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//------------------------------------------------------------------
// Modifications: (date, author, description)
//
//------------------------------------------------------------------


#include "configscreen.h"
#include <set>
#include <list>
#include <algorithm>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/imaglist.h>
#include "mainframe.h"
#include "descriptor.h"
#include "settings.h"
#include "const.h"
#include "util.h"
#include "proxy.h"
#include "testframe.h"
#include "images/box_on.xpm"
#include "images/box_off.xpm"
#include "images/box_half.xpm"
#include "images/btn_addfold32.xpm"
#include "images/btn_addfilt32.xpm"
#include "images/btn_trash32.xpm"
#include "images/btn_edit32.xpm"
#include "images/btn_ok32.xpm"
#include "images/btn_undo32.xpm"

using namespace std;


/* Events
 */
BEGIN_EVENT_TABLE(CConfigScreen, wxEvtHandler)
    EVT_BUTTON     (ID_ADDFOLDBUTTON, CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_ADDFILTBUTTON, CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_TRASHBUTTON,   CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_EDITBUTTON,    CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_REVERTBUTTON,  CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_APPLYBUTTON,   CConfigScreen::OnCommand)
    EVT_COMBOBOX   (ID_NAMEEDIT,      CConfigScreen::OnCommand)
    EVT_TEXT_ENTER (ID_NAMEEDIT,      CConfigScreen::OnCommand)
    EVT_MENU_RANGE (ID_CONFIGNEW, ID_FILTERSDECODE, CConfigScreen::OnCommand)
    EVT_TREE_STATE_IMAGE_CLICK(ID_TREE, CConfigScreen::OnTreeEvent)
    EVT_TREE_SEL_CHANGED      (ID_TREE, CConfigScreen::OnTreeEvent)
    EVT_TREE_END_LABEL_EDIT   (ID_TREE, CConfigScreen::OnTreeEvent)
    EVT_TREE_ITEM_ACTIVATED   (ID_TREE, CConfigScreen::OnTreeEvent)
    EVT_TREE_BEGIN_DRAG       (ID_TREE, CConfigScreen::OnTreeEvent)
    EVT_TREE_END_DRAG         (ID_TREE, CConfigScreen::OnTreeEvent)
END_EVENT_TABLE()


/* Constructor
 */
CConfigScreen::CConfigScreen(wxFrame* frame, wxWindow* window) :
                                CWindowContent(frame, window, wxVERTICAL) {

    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    Add(nameSizer,0,wxGROW | wxALL,0);

    pmStaticText* nameLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_CONFIG_NAME").c_str(),
        wxDefaultPosition, wxDefaultSize  );
    nameSizer->Add(nameLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxArrayString choices;
    nameEdit =  new pmComboBox(window, ID_NAMEEDIT , "",
        wxDefaultPosition, wxDefaultSize, choices,
        wxCB_DROPDOWN | wxCB_SORT );
    nameEdit->SetHelpText(settings.getMessage("CONFIG_NAME_TIP").c_str());
    nameSizer->Add(nameEdit,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxBoxSizer* centerBox = new wxBoxSizer(wxHORIZONTAL);
    Add(centerBox,1,wxGROW | wxLEFT | wxRIGHT, 5);

    wxBoxSizer* buttonBox = new wxBoxSizer(wxVERTICAL);
    centerBox->Add(buttonBox, 0, wxALIGN_TOP | wxALL, 0);

    pmBitmapButton* addfoldButton =  new pmBitmapButton(window, ID_ADDFOLDBUTTON,
        wxBitmap(btn_addfold32_xpm) );
    addfoldButton->SetHelpText(settings.getMessage("MENU_FOLDERSNEW_TIP").c_str());
    buttonBox->Add(addfoldButton,0,wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

    pmBitmapButton* addfiltButton =  new pmBitmapButton(window, ID_ADDFILTBUTTON,
        wxBitmap(btn_addfilt32_xpm) );
    addfiltButton->SetHelpText(settings.getMessage("MENU_FILTERSNEW_TIP").c_str());
    buttonBox->Add(addfiltButton,0,wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

    pmBitmapButton* editButton =  new pmBitmapButton(window, ID_EDITBUTTON,
        wxBitmap(btn_edit32_xpm) );
    editButton->SetHelpText(settings.getMessage("MENU_FILTERSEDIT_TIP").c_str());
    buttonBox->Add(editButton,0,wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

    pmBitmapButton* trashButton =  new pmBitmapButton(window, ID_TRASHBUTTON,
        wxBitmap(btn_trash32_xpm) );
    trashButton->SetHelpText(settings.getMessage("MENU_FILTERSDELETE_TIP").c_str());
    buttonBox->Add(trashButton,0,wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

    pmBitmapButton* applyButton =  new pmBitmapButton(window, ID_APPLYBUTTON,
        wxBitmap(btn_ok32_xpm) );
    applyButton->SetHelpText(settings.getMessage("MENU_CONFIGAPPLY_TIP").c_str());
    buttonBox->Add(applyButton,0,wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

    pmBitmapButton* revertButton =  new pmBitmapButton(window, ID_REVERTBUTTON,
        wxBitmap(btn_undo32_xpm) );
    revertButton->SetHelpText(settings.getMessage("MENU_CONFIGREVERT_TIP").c_str());
    buttonBox->Add(revertButton,0,wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 0);

    tree = new pmTreeCtrl(window, ID_TREE, wxDefaultPosition, wxSize(350, 250),
        wxTR_EDIT_LABELS | wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT |
        wxTR_MULTIPLE | wxTR_EXTENDED | wxTR_LINES_AT_ROOT );
    centerBox->Add(tree,1,wxGROW | wxLEFT,5);

    commentText =  new pmTextCtrl(window, ID_COMMENTTEXT, "" ,
        wxDefaultPosition, wxSize(100,60), wxTE_MULTILINE | wxTE_READONLY );
    commentText->SetHelpText(settings.getMessage("CONFIG_COMMENT_TIP").c_str());
    Add(commentText, 0,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,5);

    // Menu
    wxMenu* menuConfig = new wxMenu();
    menuConfig->Append(ID_FOLDERSNEW,
        settings.getMessage("MENU_FOLDERSNEW").c_str(),
        settings.getMessage("MENU_FOLDERSNEW_TIP").c_str());
    menuConfig->Append(ID_FILTERSNEW,
        settings.getMessage("MENU_FILTERSNEW").c_str(),
        settings.getMessage("MENU_FILTERSNEW_TIP").c_str());
    menuConfig->Append(ID_FILTERSEDIT,
        settings.getMessage("MENU_FILTERSEDIT").c_str(),
        settings.getMessage("MENU_FILTERSEDIT_TIP").c_str());
    menuConfig->AppendSeparator();
    menuConfig->Append(ID_FILTERSCHECK,
        settings.getMessage("MENU_FILTERSCHECK").c_str(),
        settings.getMessage("MENU_FILTERSCHECK_TIP").c_str());
    menuConfig->Append(ID_FILTERSDELETE,
        settings.getMessage("MENU_FILTERSDELETE").c_str(),
        settings.getMessage("MENU_FILTERSDELETE_TIP").c_str());
    menuConfig->AppendSeparator();
    menuConfig->Append(ID_FILTERSEXPORT,
        settings.getMessage("MENU_FILTERSEXPORT").c_str(),
        settings.getMessage("MENU_FILTERSEXPORT_TIP").c_str());
    menuConfig->Append(ID_FILTERSIMPORT,
        settings.getMessage("MENU_FILTERSIMPORT").c_str(),
        settings.getMessage("MENU_FILTERSIMPORT_TIP").c_str());
    menuConfig->Append(ID_FILTERSPROXOMITRON,
        settings.getMessage("MENU_FILTERSPROXOMITRON").c_str(),
        settings.getMessage("MENU_FILTERSPROXOMITRON_TIP").c_str());
    menuConfig->AppendSeparator();
    menuConfig->Append(ID_CONFIGNEW,
        settings.getMessage("MENU_CONFIGNEW").c_str(),
        settings.getMessage("MENU_CONFIGNEW_TIP").c_str());
    menuConfig->Append(ID_CONFIGDUPLICATE,
        settings.getMessage("MENU_CONFIGDUPLICATE").c_str(),
        settings.getMessage("MENU_CONFIGDUPLICATE_TIP").c_str());
    menuConfig->Append(ID_CONFIGDELETE,
        settings.getMessage("MENU_CONFIGDELETE").c_str(),
        settings.getMessage("MENU_CONFIGDELETE_TIP").c_str());
    menuConfig->AppendSeparator();
    menuConfig->Append(ID_CONFIGAPPLY,
        settings.getMessage("MENU_CONFIGAPPLY").c_str(),
        settings.getMessage("MENU_CONFIGAPPLY_TIP").c_str());
    menuConfig->Append(ID_CONFIGREVERT,
        settings.getMessage("MENU_CONFIGREVERT").c_str(),
        settings.getMessage("MENU_CONFIGREVERT_TIP").c_str());
    frame->GetMenuBar()->Insert(1, menuConfig,
        settings.getMessage("MENU_CONFIG").c_str());

    // Initialize tree
    wxImageList* images = new wxImageList(16,16,true);
    images->Add(wxBitmap(box_off_xpm));
    images->Add(wxBitmap(box_on_xpm));
    images->Add(wxBitmap(box_half_xpm));
    tree->AssignImageList(images);
    rootId = tree->AddRoot("Root", -1, -1, new CItemData(0, true));

    makeSizer();

    editWindow = new CEditScreen(&blank);
}


/* Destructor
 */
CConfigScreen::~CConfigScreen() {

    delete editWindow;
    apply(true);
    frame->GetMenuBar()->Remove(1);
}


/* Test if local variables differ from settings
 */
bool CConfigScreen::hasChanged() {

    commitText();
    return (configs != settings.configs ||
            filters != settings.filters ||
            folders != settings.folders );
}


/* Copies local variables to settings. The proxy is also restarted.
 */
void CConfigScreen::apply(bool confirm) {

    if (!hasChanged()) return;
    // Ask user before proceeding
    if (confirm) {
        int ret = wxMessageBox(settings.getMessage("APPLY_CONFIG").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }
    // Stop proxy
    CProxy::ref().closeProxyPort();
    CProxy::ref().refreshManagers();
    // Apply variables
    settings.configs = configs;
    settings.filters = filters;
    settings.folders = folders;
    settings.currentConfig = newActiveConfig;
    settings.modified = true;
    // Restart proxy
    CProxy::ref().openProxyPort();
}


/* (Re)loads settings into local variables
 */
void CConfigScreen::revert(bool confirm) {

    // Ask user before proceeding
    if (confirm && hasChanged()) {
        int ret = wxMessageBox(settings.getMessage("REVERT_CONFIG").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }

    // Load variables
    configs = settings.configs;
    filters = settings.filters;
    folders = settings.folders;
    editedConfigName = newActiveConfig = settings.currentConfig;
    
    // Populate combobox's list
    nameEdit->Clear();
    for (map<string,set<int> >::iterator it = configs.begin();
                it != configs.end(); it++)
        nameEdit->Append(it->first.c_str());
    nameEdit->SetValue(editedConfigName.c_str());
    nameEdit->SetFocus();

    // Populate tree
    populate(rootId);
}


/* Populate a tree node. Returns the state of the node.
 */
int CConfigScreen::populate(wxTreeItemId id) {

    bool states[3];
    states[0] = states[1] = states[2] = false;
    tree->DeleteChildren(id);
    CItemData* data = (CItemData*)tree->GetItemData(id);
    CFolder& folder = folders[data->id];
    for (set<int>::iterator it = folder.children.begin();
                it != folder.children.end(); it++) {
        CItemData* data2 = new CItemData(*it, true);
        wxTreeItemId id2 = tree->AppendItem(id, folders[*it].name.c_str(), 1, -1, data2);
        int state = populate(id2);
        states[state] = true;
    }
    set<int>& config = configs[editedConfigName];
    for (map<int, CFilterDescriptor>::iterator it = filters.begin();
                it != filters.end(); it++) {
        if (it->second.folder != data->id) continue;
        int state = (config.find(it->first) != config.end() ? 1 : 0);
        states[state] = true;
        CItemData* data2 = new CItemData(it->first, false, state);
        wxTreeItemId id2 = tree->AppendItem(id, it->second.title.c_str(), state, -1, data2);
        tree->SetItemBold(id2);
        if (!it->second.errorMsg.empty()) tree->SetItemTextColour(id2, *wxRED);
    }
    data->state = (!states[1] && !states[2] ? 0 : !states[0] && !states[2] ? 1 : 2);
    tree->SetItemImage(id, data->state);
    return data->state;
}


/* Recursively update tree items (states + icons)
 */
void CConfigScreen::showStates(wxTreeItemId id) {

    CItemData* data = (CItemData*)tree->GetItemData(id);
    if (!data) return;
    if (data->folder) {
        bool states[3];
        states[0] = states[1] = states[2] = false;
        wxTreeItemIdValue cookie;
        for (wxTreeItemId item = tree->GetFirstChild(id,cookie);
                          item.IsOk(); item = tree->GetNextChild(id,cookie)) {
            showStates(item);
            states[tree->GetItemImage(item)] = true;
        }
        data->state = (!states[1] && !states[2] ? 0 : !states[0] && !states[2] ? 1 : 2);
    } else {
        set<int>& config = configs[editedConfigName];
        data->state = (config.find(data->id) != config.end() ? 1 : 0);
    }
    if (data->state != tree->GetItemImage(id)) {
        tree->SetItemImage(id, data->state);
    }
}


/* Recursively update parent tree items (states + icons)
 */
void CConfigScreen::showParentStates(wxTreeItemId id) {

    if (id == rootId) return;
    for (id = tree->GetItemParent(id); id != rootId; id = tree->GetItemParent(id)) {
        bool states[3];
        states[0] = states[1] = states[2] = false;
        wxTreeItemIdValue cookie;
        for (wxTreeItemId item = tree->GetFirstChild(id,cookie);
                          item.IsOk(); item = tree->GetNextChild(id,cookie)) {
            states[tree->GetItemImage(item)] = true;
            if (states[0] && states[1] || states[2]) break;
        }
        CItemData* data = (CItemData*)tree->GetItemData(id);
        data->state = (!states[1] && !states[2] ? 0 : !states[0] && !states[2] ? 1 : 2);
        if (data->state != tree->GetItemImage(id)) {
            tree->SetItemImage(id, data->state);
        } else {
            break;
        }
    }
}


/* Recursively change item states (config + states + icons)
 */
void CConfigScreen::setStates(wxTreeItemId id, int state) {

    CItemData* data = (CItemData*)tree->GetItemData(id);
    if (!data) return;
    if (data->folder) {
        bool states[3];
        states[0] = states[1] = states[2] = false;
        wxTreeItemIdValue cookie;
        for (wxTreeItemId item = tree->GetFirstChild(id,cookie);
                          item.IsOk(); item = tree->GetNextChild(id,cookie)) {
            setStates(item, state);
            states[tree->GetItemImage(item)] = true;
        }
        data->state = (!states[1] && !states[2] ? 0 : !states[0] && !states[2] ? 1 : 2);
    } else {
        data->state = state;
        set<int>& config = configs[editedConfigName];
        if (state) {
            config.insert(data->id);
        } else {
            config.erase(data->id);
        }
    }
    if (data->state != tree->GetItemImage(id)) {
        tree->SetItemImage(id, data->state);
    }
}


/* Increments a name if it is already in the config name list
 */
string CConfigScreen::makeNewName(string oldname, string newname) {

    set<string> names;
    for (map<string, set<int> >::iterator it = configs.begin();
                it != configs.end(); it++) {
        if (it->first != oldname) {
            names.insert(it->first);
        }
    }
    while (names.find(newname) != names.end())
        CUtil::increment(newname);
    return newname;
}


/* Event handling functions
 */
void CConfigScreen::OnCommand(wxCommandEvent& event) {

    switch (event.GetId()) {
    case ID_NAMEEDIT:
        {
            if (event.GetEventType() == wxEVT_COMMAND_TEXT_ENTER) {

                string newName = nameEdit->GetValue().c_str();
                CUtil::trim(newName);
                if (newName.empty())
                    newName = settings.getMessage("CONFIG_NEW_NAME");
                newName = makeNewName(editedConfigName, newName);
                if (newName != editedConfigName) {
                    // The user renamed the configuration
                    if (newActiveConfig == editedConfigName)
                        newActiveConfig = newName;
                    configs[newName] = configs[editedConfigName];
                    configs.erase(editedConfigName);
                    nameEdit->SetValue(newName.c_str());
                    nameEdit->Append(newName.c_str());
                    nameEdit->Delete(nameEdit->FindString(editedConfigName.c_str()));
                    editedConfigName = newName;
                }
            } else if (event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED) {

                // The user changed current configuration
                editedConfigName = event.GetString();
                showStates(rootId);
            }
            break;
        }
    case ID_CONFIGNEW:
        {
            commitText();
            editedConfigName = makeNewName("", settings.getMessage("CONFIG_NEW_NAME"));
            configs[editedConfigName];
            nameEdit->Append(editedConfigName.c_str());
            nameEdit->SetValue(editedConfigName.c_str());
            showStates(rootId);
            break;
        }
    case ID_CONFIGDUPLICATE:
        {
            commitText();
            string newName = makeNewName("", editedConfigName);
            configs[newName] = configs[editedConfigName];
            nameEdit->Append(newName.c_str());
            nameEdit->SetValue(newName.c_str());
            editedConfigName = newName;
            break;
        }
    case ID_CONFIGDELETE:
        {
            commitText();
            configs.erase(editedConfigName);
            nameEdit->Delete(nameEdit->FindString(editedConfigName.c_str()));
            if (configs.empty()) {
                string newName = settings.getMessage("CONFIG_NEW_NAME");
                configs[newName];
                nameEdit->Append(newName.c_str());
            }
            if (editedConfigName == newActiveConfig)
                newActiveConfig = configs.begin()->first;
            editedConfigName = configs.begin()->first;
            nameEdit->SetValue(editedConfigName.c_str());
            showStates(rootId);
            break;
        }
    case ID_REVERTBUTTON:
    case ID_CONFIGREVERT:
        {
            commitText();
            revert(true);
            break;
        }
    case ID_APPLYBUTTON:
    case ID_CONFIGAPPLY:
        {
            commitText();
            apply(false);
            break;
        }
    case ID_FILTERSCHECK:
        {
            checkSelection();
            break;
        }
    case ID_EDITBUTTON:
    case ID_FILTERSEDIT:
        {
            CItemData* data = (CItemData*)tree->GetItemData(currentId);
            if (data && !data->folder) {
                editWindow->Show();
                editWindow->Raise();
            }
            break;
        }
    case ID_ADDFOLDBUTTON:
    case ID_FOLDERSNEW:
        {
            wxTreeItemId folderId = getSelectionId();
            CItemData* data = (CItemData*)tree->GetItemData(folderId);
            if (!data->folder) {
                folderId = tree->GetItemParent(folderId);
                data = (CItemData*)tree->GetItemData(folderId);
            }
            int id = (folders.empty() ? 1 : folders.rbegin()->second.id + 1);
            string name = settings.getMessage("DEFAULT_FOLDER_NAME");
            CFolder newfolder(id, name, data->id);
            folders[id] = newfolder;
            folders[data->id].children.insert(id);
            data = new CItemData(id, true, 0);
            wxTreeItemId item = tree->PrependItem(folderId, name.c_str(), 0, -1, data);
            tree->UnselectAll();
            tree->EnsureVisible(item);
            tree->SelectItem(item);
            selectionId = item;
            refreshEditWindow();
            tree->EditLabel(item);
            break;
        }
    case ID_ADDFILTBUTTON:
    case ID_FILTERSNEW:
        {
            wxTreeItemId folderId = getSelectionId();
            CItemData* data = (CItemData*)tree->GetItemData(folderId);
            if (!data->folder) {
                folderId = tree->GetItemParent(folderId);
                data = (CItemData*)tree->GetItemData(folderId);
            }
            CFilterDescriptor desc;
            desc.id = (filters.empty() ? 1 : filters.rbegin()->second.id + 1);
            desc.folder = data->id;
            desc.title = settings.getMessage("DEFAULT_FILTER_NAME");
            filters[desc.id] = desc;
            data = new CItemData(desc.id, false, 0);
            wxTreeItemId item = tree->AppendItem(folderId, desc.title.c_str(), 0, -1, data);
            tree->SetItemBold(item);
            tree->SetItemTextColour(item, *wxRED);
            tree->UnselectAll();
            tree->EnsureVisible(item);
            tree->SelectItem(item);
            selectionId = item;
            refreshEditWindow();
            editWindow->Show();
            editWindow->Raise();
            break;
        }
    case ID_FILTERSIMPORT:
        {
            string text = CUtil::getClipboard();
            if (!text.empty()) importFilters(text);
            break;
        }
    case ID_FILTERSPROXOMITRON:
        {
            string text = CUtil::getClipboard();
            if (!text.empty()) importFilters(text, true);
            break;
        }
    case ID_FILTERSEXPORT:
        {
            CUtil::setClipboard(exportSelection());
            break;
        }
    case ID_TRASHBUTTON:
    case ID_FILTERSDELETE:
        {
            CUtil::setClipboard(exportSelection());
            deleteSelection();
            break;
        }
    default:
        event.Skip();
    }
}


/* Tree event management
 */
void CConfigScreen::OnTreeEvent(wxTreeEvent& event) {

    WXTYPE evt = event.GetEventType();
    wxTreeItemId id = event.GetItem();
    CItemData* data = (CItemData*)tree->GetItemData(id);

    if (evt == wxEVT_COMMAND_TREE_STATE_IMAGE_CLICK) {

        // An item is checked/unchecked
        checkSelection();

    } else if (evt == wxEVT_COMMAND_TREE_SEL_CHANGED) {

        // Selection changed
        selectionId = id;
        refreshEditWindow();

    } else if (evt == wxEVT_COMMAND_TREE_END_LABEL_EDIT) {

        // The user changed the folder name/filter title
        if (!event.IsEditCancelled()) {
            string text = event.GetLabel().c_str();
            CUtil::trim(text);
            if (text.empty()) {
                event.Veto();
            } else {
                if (data->folder) {
                    folders[data->id].name = text;
                    folders[data->id].defaultFolder = 0;
                } else {
                    filters[data->id].title = text;
                    filters[data->id].defaultFilter = 0;
                    editWindow->setCurrent(&filters[data->id]);
                }
            }
        }

    } else if (evt == wxEVT_COMMAND_TREE_ITEM_ACTIVATED) {

        // The user double-clicked on the current item
        if (data && !data->folder) {
            editWindow->Show();
            editWindow->Raise();
        }

    } else if (evt == wxEVT_COMMAND_TREE_BEGIN_DRAG) {

        // The user started drag&drop
        tree->GetSelections(dragNDrop);
        tree->UnselectAll();
        event.Allow();
        
    } else if (evt == wxEVT_COMMAND_TREE_END_DRAG) {

        // The user ended drag&drop
        
        int count = dragNDrop.GetCount();
        for (int i = 0; i < count; i++)
            tree->SelectItem(dragNDrop[i]);
        string text = exportSelection();
        deleteSelection(false);
        selectionId = event.GetItem();
        if (!selectionId.IsOk() || !tree->GetItemData(selectionId))
            selectionId = rootId;
        importFilters(text);
        showStates(rootId);

    } else {

        event.Skip();
    }
}


/* Unselects the content of a folder (but not the folder itself)
 */
void CConfigScreen::unselectFolder(wxTreeItemId id) {

    wxTreeItemIdValue cookie;
    for (wxTreeItemId item = tree->GetFirstChild(id,cookie);
                      item.IsOk(); item = tree->GetNextChild(id,cookie)) {
        CItemData* data = (CItemData*)tree->GetItemData(item);
        if (data->folder) {
            unselectFolder(item);
        }
        if (tree->IsSelected(item)) tree->UnselectItem(item);
    }
}


/* Simplifies the selection by unselecting items in selected folders
 */
void CConfigScreen::simplifySelection() {

    wxArrayTreeItemIds list;
    tree->GetSelections(list);
    int count = list.GetCount();
    for (int i = 0; i < count; i++) {
        CItemData* data = (CItemData*)tree->GetItemData(list[i]);
        if (data->folder) unselectFolder(list[i]);
    }
}


/* Deletes folders and filters.
 * configs are refreshed if update == true
 * The tree items are not deleted.
 */
void CConfigScreen::deleteItem(wxTreeItemId id, bool update) {

    CItemData* data = (CItemData*)tree->GetItemData(id);
    if (!data) return;
    if (data->folder) {
        wxTreeItemIdValue cookie;
        for (wxTreeItemId item = tree->GetFirstChild(id,cookie);
                          item.IsOk(); item = tree->GetNextChild(id,cookie)) {
            deleteItem(item, update);
        }
        folders[folders[data->id].parent].children.erase(data->id);
        folders.erase(data->id);
    } else {
        filters.erase(data->id);
        if (update) {
            for (map<string, set<int> >::iterator it = configs.begin();
                        it != configs.end(); it++) {
                it->second.erase(data->id);
            }
        }
    }
}


/* Deletes the selection.
 * configs and states are refreshed only if update == true
 */
void CConfigScreen::deleteSelection(bool update) {

    selectionId = rootId;
    refreshEditWindow();
    simplifySelection();
    wxArrayTreeItemIds list;
    tree->GetSelections(list);
    int count = list.GetCount();
    for (int i = 0; i < count; i++) {
        deleteItem(list[i], update);
        tree->Delete(list[i]);
    }
    if (update) showStates(rootId);
}


/* Exports a filter or a whole folder
 */
void CConfigScreen::exportItem(wxTreeItemId id, int root, stringstream& out) {

    CItemData* data = (CItemData*)tree->GetItemData(id);
    if (!data) return;
    if (data->folder) {
        wxTreeItemIdValue cookie;
        for (wxTreeItemId item = tree->GetFirstChild(id,cookie);
                          item.IsOk(); item = tree->GetNextChild(id,cookie)) {
            exportItem(item, root, out);
        }
    } else {
        out << filters[data->id].exportFilter(folders, root);
    }
}


/* Exports all filters within selection
 */
string CConfigScreen::exportSelection() {

    simplifySelection();
    stringstream out;
    wxArrayTreeItemIds list;
    tree->GetSelections(list);
    int count = list.GetCount();
    for (int i = 0; i < count; i++) {
        CItemData* data = (CItemData*)tree->GetItemData(tree->GetItemParent(list[i]));
        exportItem(list[i], data->id, out);
    }
    return out.str();
}


/* Imports filters into the current folder
 */
void CConfigScreen::importFilters(const string& text, bool proxo) {

    wxTreeItemId folderId = getSelectionId();
    CItemData* data = (CItemData*)tree->GetItemData(folderId);
    if (!data->folder) {
        folderId = tree->GetItemParent(folderId);
        data = (CItemData*)tree->GetItemData(folderId);
    }
    if (proxo) {
        CFilterDescriptor::importProxomitron(text, folders, filters, data->id);
    } else {
        CFilterDescriptor::importFilters(text, folders, filters, data->id);
    }
    populate(folderId);
    showParentStates(folderId);
}


/* Refresh the edit window with the given item
 */
void CConfigScreen::refreshEditWindow() {

    // update old edited filter
    CItemData* prev = (CItemData*)tree->GetItemData(currentId);
    if (prev && !prev->folder) {
        tree->SetItemText(currentId, filters[prev->id].title.c_str());
        if (!filters[prev->id].errorMsg.empty()) {
            tree->SetItemTextColour(currentId, *wxRED);
        } else {
            tree->SetItemTextColour(currentId, *wxBLACK);
        }
    }
    // update comment and edit window
    wxTreeItemId id = getSelectionId();
    CItemData* data = (CItemData*)tree->GetItemData(id);
    if (!data || data->folder) {
        commentText->SetValue("");
    } else {
        commentText->SetValue(filters[data->id].comment.c_str());
        currentId = id;
        editWindow->setCurrent(&filters[data->id]);
    }
}


/* Change check status for selected filters/folders
 */
void CConfigScreen::checkSelection() {

    wxArrayTreeItemIds list;
    tree->GetSelections(list);
    int count = list.GetCount();
    int state = -1; // (not decided yet)
    for (int i = 0; i < count; i++) {
        CItemData* data = (CItemData*)tree->GetItemData(list[i]);
        if (!data) continue;
        if (state < 0) state = (data->state != 1 ? 1 : 0);
        if (data->state != state) {
            setStates(list[i], state);
        }
    }
    showStates(rootId);
}


/* Returns the selected item's ID.
 */
wxTreeItemId CConfigScreen::getSelectionId() {

    wxArrayTreeItemIds list;
    tree->GetSelections(list);
    CItemData* data = (CItemData*)tree->GetItemData(selectionId);
    if (list.IsEmpty() || !data)
        selectionId = rootId;
    return selectionId;
}

