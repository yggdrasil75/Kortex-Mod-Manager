#pragma once
#include "stdafx.h"
#include "DataType.h"
#include "ITypeDetector.h"
#include "ItemOptions.h"
#include "ItemGroup.h"

namespace Kortex::GameConfig
{
	class Definition
	{
		private:
			wxString m_ID;
			wxString m_FilePath;
			std::vector<DataType> m_DataTypes;
			std::vector<std::unique_ptr<ITypeDetector>> m_TypeDetectors;

			std::unordered_map<wxString, std::unique_ptr<ItemGroup>> m_Groups;
			ItemOptions m_Options;
			bool m_IsLoaded = false;

		private:
			void LoadGroups(const KxXMLNode& groupsNode);
			template<class TItems, class TFunctor> static void ForEachItem(TItems&& items, TFunctor&& func)
			{
				for (auto& [id, item]: items)
				{
					func(*item);
				}
			}

		public:
			Definition(const wxString& id, const wxString& filePath)
				:m_ID(id), m_FilePath(filePath)
			{
			}
			virtual ~Definition();

		public:
			wxString GetID() const
			{
				return m_ID;
			}
			wxString GetFilePath() const
			{
				return m_FilePath;
			}
			const ItemOptions& GetOptions() const
			{
				return m_Options;
			}
			bool Load();

			ItemGroup* GetGroupByID(const wxString& id)
			{
				auto it = m_Groups.find(id);
				return it != m_Groups.end() ? it->second.get() : nullptr;
			}
			template<class TFunctor> void ForEachGroup(TFunctor&& func) const
			{
				ForEachItem(m_Groups, func);
			}
			template<class TFunctor> void ForEachGroup(TFunctor&& func)
			{
				ForEachItem(m_Groups, func);
			}
	};
}