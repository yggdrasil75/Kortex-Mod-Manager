#include "stdafx.h"
#include "SimpleItem.h"
#include "Utility/KAux.h"

namespace Kortex::GameConfig
{
	bool SimpleItem::Create(const KxXMLNode& itemNode)
	{
		return IsOK();
	}

	void SimpleItem::Clear()
	{
		m_Value.MakeNull();
	}
	void SimpleItem::Read(const ISource& source)
	{
		source.ReadValue(*this, m_Value);
	}
	void SimpleItem::Write(ISource& source) const
	{
		source.WriteValue(*this, m_Value);
	}

	SimpleItem::SimpleItem(ItemGroup& group, const KxXMLNode& itemNode)
		:IExtendInterface(group, itemNode)
	{
	}
	SimpleItem::SimpleItem(ItemGroup& group, bool isUnknown)
		:IExtendInterface(group), m_IsUnknown(isUnknown)
	{
	}

	wxString SimpleItem::GetStringRepresentation(ColumnID id) const
	{
		if (id == ColumnID::Value)
		{
			return m_Value.As<wxString>();
		}
		return Item::GetStringRepresentation(id);
	}

	wxAny SimpleItem::GetValue(const KxDataView2::Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Path:
			{
				return GetFullPath();
			}
			case ColumnID::Name:
			{
				return GetLabel();
			}
			case ColumnID::Type:
			{
				return GetTypeID().ToString();
			}
			case ColumnID::Value:
			{
				if (!m_CachedViewData)
				{
					auto FormatValue = [this](const ItemValue& value)
					{
						wxString serializedValue = value.Serialize(*this);

						if (serializedValue.IsEmpty())
						{
							if (value.GetType().IsString())
							{
								return KAux::MakeBracketedLabel(GetManager().GetTranslator().GetString(wxS("ConfigManager.View.EmptyStringValue")));
							}
							else
							{
								return KAux::MakeNoneLabel();
							}
						}
						return serializedValue;
					};

					const SampleValue* sampleValue = GetSamples().FindSampleByValue(m_Value);
					if (sampleValue && sampleValue->HasLabel())
					{
						m_CachedViewData = KxString::Format(wxS("%1 - %2"), FormatValue(m_Value), sampleValue->GetLabel());
					}
					else
					{
						m_CachedViewData = FormatValue(m_Value);
					}
				}
				return *m_CachedViewData;
			}
		}
		return {};
	}
	wxAny SimpleItem::GetEditorValue(const KxDataView2::Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Value:
			{
				return m_Value.AsAny();
			}
		}
		return {};
	}
	KxDataView2::Renderer& SimpleItem::GetRenderer(const KxDataView2::Column& column) const
	{
		return m_Renderer;
	}
	KxDataView2::Editor* SimpleItem::GetEditor(const KxDataView2::Column& column) const
	{
		return nullptr;
	}
	bool SimpleItem::GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const
	{
		return false;
	}
}
