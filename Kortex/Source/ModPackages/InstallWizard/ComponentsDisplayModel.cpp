#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/PackageManager.hpp>
#include "ComponentsDisplayModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>

namespace
{
	enum ColumnID
	{
		Expander,
		Name,
	};
}

namespace Kortex::InstallWizard
{
	void ComponentsDisplayModel::OnInitControl()
	{
		/* View */
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &ComponentsDisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_HOVERED, &ComponentsDisplayModel::OnHotTrackItem, this);

		/* Columns */
		GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, ColumnID::Expander, KxDATAVIEW_CELL_INERT, 0, KxDV_COL_HIDDEN);
		GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE);
	}

	bool ComponentsDisplayModel::IsContainer(const KxDataViewItem& item) const
	{
		if (const ComponentsDisplayModelNode* node = GetNode(item))
		{
			return node->IsGroup() && node->HasChildren();
		}
		return false;
	}
	KxDataViewItem ComponentsDisplayModel::GetParent(const KxDataViewItem& item) const
	{
		if (const ComponentsDisplayModelNode* node = GetNode(item))
		{
			if (node->IsEntry() && node->HasParentNode())
			{
				return MakeItem(node->GetParentNode());
			}
		}
		return KxDataViewItem();
	}
	void ComponentsDisplayModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		// Root item, read groups
		if (item.IsTreeRootItem())
		{
			for (const ComponentsDisplayModelNode& node: m_DataVector)
			{
				if (node.IsGroup())
				{
					children.push_back(MakeItem(node));
				}
			}
			return;
		}

		// Group item, read entries
		const ComponentsDisplayModelNode* node = GetNode(item);
		if (const KPPCGroup* group = node->GetGroup())
		{
			for (size_t i = node->GetBegin(); i < node->GetSize(); i++)
			{
				if (m_DataVector[i].IsEntry())
				{
					children.push_back(MakeItem(m_DataVector[i]));
				}
			}
		}
	}

	bool ComponentsDisplayModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attibutes, KxDataViewCellState cellState) const
	{
		const ComponentsDisplayModelNode* node = GetNode(item);
		if (node)
		{
			if (const KPPCGroup* group = node->GetGroup())
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					{
						attibutes.SetForegroundColor(KxUtility::GetThemeColor_Caption(GetView()));
						attibutes.SetBold(true);
						return true;
					}
				};
			}
			else if (const KPPCEntry* entry = node->GetEntry())
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					{
						switch (entry->GetTDCurrentValue())
						{
							case KPPC_DESCRIPTOR_RECOMMENDED:
							{
								attibutes.SetItalic(true);
								break;
							}
						}
					}
				};
			}
		}
		return false;
	}
	void ComponentsDisplayModel::GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const ComponentsDisplayModelNode* node = GetNode(item))
		{
			if (const KPPCGroup* group = node->GetGroup())
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					{
						data = wxString::Format("%s (%s)", group->GetName(), GetSelectionModeString(*group));
						break;
					}
				};
			}
			else if (const KPPCEntry* entry = node->GetEntry())
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					{
						KxDataViewBitmapTextToggleValue value(node->IsChecked(), GetToggleType(node->GetParentNode()->GetGroup()->GetSelectionMode()));
						value.SetBitmap(GetImageByTypeDescriptor(entry->GetTDCurrentValue()));
						value.SetText(entry->GetName());
						data = value;
						break;
					}
				};
			}
		}
	}
	bool ComponentsDisplayModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		ComponentsDisplayModelNode* entryNode = GetNode(item);
		if (entryNode)
		{
			if (const KPPCEntry* entry = entryNode->GetEntry())
			{
				const KPPCGroup* group = entryNode->GetParentNode()->GetGroup();
				KPPCSelectionMode selMode = group->GetSelectionMode();
				KPPCTypeDescriptor typeDescriptor = entry->GetTDCurrentValue();

				if (column->GetID() == ColumnID::Name)
				{
					auto CountChecked = [this, entryNode]()
					{
						size_t checkedCount = 0;
						for (const ComponentsDisplayModelNode* node: GetGroupNodes(entryNode->GetParentNode()))
						{
							if (node->IsChecked())
							{
								checkedCount++;
							}
						}
						return checkedCount;
					};
					auto SetAllChecked = [this, entryNode](bool bCheck)
					{
						for (ComponentsDisplayModelNode* node: GetGroupNodes(entryNode->GetParentNode()))
						{
							node->SetChecked(false);
							NodeChanged(node);
						}
					};

					switch (selMode)
					{
						case KPPC_SELECT_ANY:
						{
							entryNode->ToggleCheck();
							NodeChanged(entryNode);
							return true;
						}
						case KPPC_SELECT_EXACTLY_ONE:
						{
							// Is this entry is going to be checked, then uncheck all entries in this group
							// and check this one.
							if (!entryNode->IsChecked())
							{
								SetAllChecked(false);

								if (CountChecked() == 0)
								{
									entryNode->SetChecked(true);
									NodeChanged(entryNode);
								}
								return true;
							}
							break;
						}
						case KPPC_SELECT_AT_LEAST_ONE:
						{
							if (!entryNode->IsChecked())
							{
								entryNode->SetChecked(true);
								NodeChanged(entryNode);
							}
							else if (CountChecked() > 1)
							{
								entryNode->SetChecked(false);
								NodeChanged(entryNode);
							}
							return true;
						}
						case KPPC_SELECT_AT_MOST_ONE:
						{
							if (!entryNode->IsChecked())
							{
								SetAllChecked(false);

								if (CountChecked() == 0)
								{
									entryNode->SetChecked(true);
									NodeChanged(entryNode);
								}
							}
							else
							{
								entryNode->SetChecked(false);
								NodeChanged(entryNode);
							}
							return true;
						}
						case KPPC_SELECT_ALL:
						{
							entryNode->SetChecked(true);
							NodeChanged(entryNode);
						}
					};
				}
			}
		}
		return false;
	}
	bool ComponentsDisplayModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const ComponentsDisplayModelNode* node = GetNode(item);
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return node && node->IsEntry() && !node->IsRequiredEntry();
			}
		};
		return true;
	}

	wxBitmap ComponentsDisplayModel::GetImageByTypeDescriptor(KPPCTypeDescriptor type) const
	{
		switch (type)
		{
			case KPPC_DESCRIPTOR_NOT_USABLE:
			{
				return ImageProvider::GetBitmap(ImageResourceID::CrossCircleFrame);
			}
			case KPPC_DESCRIPTOR_COULD_BE_USABLE:
			{
				return ImageProvider::GetBitmap(ImageResourceID::Exclamation);
			}
		};
		return wxNullBitmap;
	}
	wxString ComponentsDisplayModel::GetMessageTypeDescriptor(KPPCTypeDescriptor type) const
	{
		return KTr("PackageCreator.TypeDescriptor." + KPackageProjectComponents::TypeDescriptorToString(type));
	}
	KxDataViewBitmapTextToggleValue::ToggleType ComponentsDisplayModel::GetToggleType(KPPCSelectionMode mode) const
	{
		switch (mode)
		{
			case KPPC_SELECT_EXACTLY_ONE:
			case KPPC_SELECT_AT_MOST_ONE:
			{
				return KxDataViewBitmapTextToggleValue::RadioBox;
			}
		};
		return KxDataViewBitmapTextToggleValue::CheckBox;
	}
	const wxString& ComponentsDisplayModel::GetSelectionModeString(const KPPCGroup& group) const
	{
		static const wxString ms_Select = KTr("Generic.Select");
		auto MakeString = [](KPPCSelectionMode mode) -> wxString
		{
			return ms_Select + ' ' + KxString::MakeLower(KPackageProjectComponents::SelectionModeToTranslation(mode));
		};

		static const wxString ms_Any = MakeString(KPPC_SELECT_ANY);
		static const wxString ms_ExactlyOne = MakeString(KPPC_SELECT_EXACTLY_ONE);
		static const wxString ms_AtLeastOne = MakeString(KPPC_SELECT_AT_LEAST_ONE);
		static const wxString ms_AtMostOne = MakeString(KPPC_SELECT_AT_MOST_ONE);
		static const wxString ms_All = MakeString(KPPC_SELECT_ALL);

		switch (group.GetSelectionMode())
		{
			case KPPC_SELECT_EXACTLY_ONE:
			{
				return ms_ExactlyOne;
			}
			case KPPC_SELECT_AT_LEAST_ONE:
			{
				return ms_AtLeastOne;
			}
			case KPPC_SELECT_AT_MOST_ONE:
			{
				return ms_AtMostOne;
			}
			case KPPC_SELECT_ALL:
			{
				return ms_All;
			}
		};
		return ms_Any;
	}
	ComponentsDisplayModelNode::RefVector ComponentsDisplayModel::GetGroupNodes(const ComponentsDisplayModelNode* groupNode)
	{
		ComponentsDisplayModelNode::RefVector items;
		if (groupNode && groupNode->IsGroup())
		{
			for (size_t i = groupNode->GetBegin(); i < groupNode->GetSize(); i++)
			{
				if (m_DataVector[i].IsEntry())
				{
					items.push_back(&m_DataVector[i]);
				}
			}
		}
		return items;
	}
	bool ComponentsDisplayModel::IsEntryShouldBeChecked(const KPPCEntry* entry) const
	{
		KPPCTypeDescriptor typeDescriptor = entry->GetTDCurrentValue();
		if (typeDescriptor == KPPC_DESCRIPTOR_REQUIRED || typeDescriptor == KPPC_DESCRIPTOR_RECOMMENDED)
		{
			return true;
		}
		else
		{
			return std::any_of(m_CheckedEntries.begin(), m_CheckedEntries.end(), [entry](const KPPCEntry* v)
			{
				return v == entry;
			});
		}
	}

	void ComponentsDisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		if (const ComponentsDisplayModelNode* node = GetNode(item))
		{
			if (const KPPCGroup* group = node->GetGroup())
			{
				if (GetView()->IsExpanded(item))
				{
					GetView()->Collapse(item);
				}
				else
				{
					GetView()->Expand(item);
				}
			}
			else if (const KPPCEntry* entry = node->GetEntry())
			{
				KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr(KxID_INFO), GetMessageTypeDescriptor(entry->GetTDCurrentValue()), KxBTN_OK, KxICON_INFORMATION);
				dialog.ShowModal();
			}
		}
	}
	void ComponentsDisplayModel::OnHotTrackItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		if (const ComponentsDisplayModelNode* node = GetNode(item))
		{
			if (const KPPCEntry* entry = node->GetEntry())
			{
				m_HotItem = entry;
				return;
			}
		}
		m_HotItem = nullptr;
	}

	ComponentsDisplayModel::ComponentsDisplayModel()
	{
		SetDataViewFlags(KxDV_NO_HEADER);
	}

	size_t ComponentsDisplayModel::GetItemsCount() const
	{
		size_t count = 0;
		if (m_Step)
		{
			for (const auto& group: m_Step->GetGroups())
			{
				count += group->GetEntries().size() + 1;
			}
		}
		return count;
	}
	void ComponentsDisplayModel::SetDataVector()
	{
		m_DataVector.clear();
		m_CheckedEntries.clear();
		m_ComponentsInfo = nullptr;
		m_Step = nullptr;

		ItemsCleared();
		GetView()->Disable();
	}
	void ComponentsDisplayModel::SetDataVector(const KPackageProjectComponents* compInfo, const KPPCStep* step, const KPPCEntry::RefVector& checkedEntries)
	{
		SetDataVector();

		m_ComponentsInfo = compInfo;
		m_Step = step;
		m_CheckedEntries = checkedEntries;

		RefreshItems();
		GetView()->Enable();
	}
	void ComponentsDisplayModel::RefreshItems()
	{
		m_DataVector.clear();
		m_DataVector.reserve(GetItemsCount());
		ItemsCleared();

		KxDataViewItem selection;
		KxDataViewItem::Vector groupItems;
		groupItems.reserve(m_Step->GetGroups().size());
		for (const auto& group: m_Step->GetGroups())
		{
			ComponentsDisplayModelNode& groupNode = m_DataVector.emplace_back(*group);

			size_t groupSize = group->GetEntries().size();
			groupNode.SetBounds(m_DataVector.size(), (m_DataVector.size() - 1) + groupSize + 1);

			KxDataViewItem groupItem = MakeItem(groupNode);
			groupItems.push_back(groupItem);

			KxDataViewItem::Vector entryItems;
			entryItems.reserve(group->GetEntries().size());
			size_t checkedCount = 0;
			for (const auto& entry: group->GetEntries())
			{
				ComponentsDisplayModelNode& entryNode = m_DataVector.emplace_back(*entry);
				entryNode.SetParentNode(groupNode);

				KxDataViewItem entryItem = MakeItem(entryNode);
				entryItems.push_back(entryItem);
				if (!selection.IsOK())
				{
					selection = entryItem;
				}

				// Check entries of this group if all of them needs to be checked.
				// Or if this is required or recommended item.
				if (group->GetSelectionMode() == KPPC_SELECT_ALL || IsEntryShouldBeChecked(entry.get()))
				{
					checkedCount++;
					entryNode.SetChecked(true);
				}
			}

			// if no items was checked and at least one of them needs to be checked (by selection mode),
			// check the first one.
			if (checkedCount == 0 && !entryItems.empty())
			{
				switch (group->GetSelectionMode())
				{
					case KPPC_SELECT_AT_LEAST_ONE:
					case KPPC_SELECT_EXACTLY_ONE:
					{
						GetNode(entryItems[0])->SetChecked(true);
						break;
					}
				};
			}
			ItemsAdded(groupItem, entryItems);
		}
		ItemsAdded(KxDataViewItem(), groupItems);

		// Expand all groups
		for (const ComponentsDisplayModelNode& node: m_DataVector)
		{
			if (node.IsGroup())
			{
				GetView()->Expand(MakeItem(node));
			}
		}

		// Select first entry item
		SelectItem(selection);
		GetView()->SetFocus();
	}
	bool ComponentsDisplayModel::OnLeaveStep(KPPCEntry::RefVector& checkedEntries)
	{
		checkedEntries = GetCheckedEntries();
		return true;
	}

	const KPPCEntry* ComponentsDisplayModel::GetSelectedEntry() const
	{
		ComponentsDisplayModelNode* node = GetNode(GetView()->GetSelection());
		if (node)
		{
			return node->GetEntry();
		}
		return nullptr;
	}

	KxDataViewItem ComponentsDisplayModel::MakeItem(const ComponentsDisplayModelNode* node) const
	{
		return KxDataViewItem(node);
	}
	ComponentsDisplayModelNode* ComponentsDisplayModel::GetNode(const KxDataViewItem& item) const
	{
		return item.GetValuePtr<ComponentsDisplayModelNode>();
	}
	KPPCEntry::RefVector ComponentsDisplayModel::GetCheckedEntries() const
	{
		KPPCEntry::RefVector entries;
		for (const ComponentsDisplayModelNode& node: m_DataVector)
		{
			if (node.IsEntry() && node.IsChecked())
			{
				entries.push_back(const_cast<KPPCEntry*>(node.GetEntry()));
			}
		}
		return entries;
	}
}