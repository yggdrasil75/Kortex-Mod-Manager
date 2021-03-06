#pragma once
#include "stdafx.h"
#include "IDisplayModel.h"

namespace Kortex::PluginManager
{
	class BethesdaDisplayModel: public IDisplayModel
	{
		public:
			enum BSColumnID
			{
				MIN = IDisplayModel::ColumnID::MIN_USER,

				Index
			};

		protected:
			virtual void OnInitControl() override;

		public:
			virtual void GetValue(wxAny& value, const IGamePlugin& plugin, const KxDataViewColumn* column) const override;
			virtual bool SetValue(const wxAny& value, IGamePlugin& plugin, const KxDataViewColumn* column) override;

			virtual bool IsEnabled(const IGamePlugin& plugin, const KxDataViewColumn* column) const override;
			virtual bool IsEditorEnabled(const IGamePlugin& plugin, const KxDataViewColumn* column) const override;

			virtual bool GetAttributes(const IGamePlugin& plugin, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
			virtual bool Compare(const IGamePlugin& pluginLeft, const IGamePlugin& pluginRight, const KxDataViewColumn* column) const override;

		public:
			wxString GetPartOfName(const IGamePlugin& plugin) const;
			wxString GetPluginAuthor(const IGamePlugin& plugin) const;
	};
}
