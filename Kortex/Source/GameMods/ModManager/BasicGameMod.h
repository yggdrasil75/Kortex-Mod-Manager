#pragma once
#include "stdafx.h"
#include "GameMods/IGameMod.h"
#include "GameMods/FileTreeNode.h"
#include "GameMods/ModTagStore.h"
#include "GameMods/ModProvider/Store.h"
#include "Network/NetworkConstants.h"
#include "KLabeledValue.h"
#include "KWithBitmap.h"
#include <KxFramework/KxVersion.h>
#include "KAux.h"
#include "KImageProvider.h"

namespace Kortex
{
	class IModManager;
	class IModDispatcher;
}

namespace Kortex::ModManager
{
	class FixedGameMod;
	class PriorityGroup;

	class BasicGameMod: public RTTI::IImplementation<IGameMod, IGameModWithImage>
	{
		private:
			wxString m_Signature;
			wxString m_ID;
			wxString m_Name;
			wxString m_Author;
			KxVersion m_Version;

			bool m_IsDescriptionChanged = false;
			mutable wxString m_Description;

			ModProvider::Store m_ProviderStore;
			ModTagStore m_TagStore;
			wxString m_PriorityGroupTag;

			wxDateTime m_TimeInstall;
			wxDateTime m_TimeUninstall;
			wxString m_PackageFile;
			wxString m_LinkLocation;

			FileTreeNode m_FileTree;

			KxColor m_Color;
			bool m_IsActive = false;

		protected:
			bool IsInstalledReal() const;

		public:
			bool IsOK() const;

			void CreateAllFolders();
			bool Save() override;

			bool LoadUsingSignature(const wxString& signature) override;
			bool LoadUsingID(const wxString& id) override;
			bool CreateFromProject(const KPackageProject& config) override;

			wxString GetSignature() const override
			{
				return m_Signature;
			}
			wxString GetID() const override
			{
				return m_ID;
			}
			void SetID(const wxString& id) override
			{
				m_ID = id;
				m_Signature = GetSignatureFromID(id);
			}
		
			wxString GetName() const override
			{
				if (!m_Name.IsEmpty())
				{
					return m_Name;
				}
				return m_ID;
			}
			void SetName(const wxString& value) override
			{
				m_Name = value;
			}
		
			wxString GetAuthor() const override
			{
				return m_Author;
			}
			void SetAuthor(const wxString& value) override
			{
				m_Author = value;
			}

			KxVersion GetVersion() const override
			{
				return m_Version;
			}
			void SetVersion(const KxVersion& value) override
			{
				m_Version = value;
			}
		
			bool IsDescriptionChanged() const
			{
				return m_IsDescriptionChanged;
			}
			wxString GetDescription() const override;
			void SetDescription(const wxString& value);

			wxDateTime GetInstallTime() const override
			{
				return m_TimeInstall;
			}
			void SetInstallTime(const wxDateTime& date) override
			{
				m_TimeInstall = date;
			}

			wxDateTime GetUninstallTime() const override
			{
				return m_TimeUninstall;
			}
			void SetUninstallTime(const wxDateTime& date) override
			{
				m_TimeUninstall = date;
			}
		
			const ModProvider::Store& GetProviderStore() const override
			{
				return m_ProviderStore;
			}
			ModProvider::Store& GetProviderStore() override
			{
				return m_ProviderStore;
			}

			const ModTagStore& GetTagStore() const override
			{
				return m_TagStore;
			}
			ModTagStore& GetTagStore() override
			{
				return m_TagStore;
			}

			wxString GetPriorityGroupTag() const override
			{
				return m_PriorityGroupTag;
			}
			void SetPriorityGroupTag(const wxString& value)
			{
				m_PriorityGroupTag = value;
			}

			wxString GetPackageFile() const override
			{
				return m_PackageFile;
			}
			void SetPackageFile(const wxString& value) override
			{
				m_PackageFile = value;
			}
		
			const FileTreeNode& GetFileTree() const override;
			void ClearFileTree() override;
			void UpdateFileTree() override;

			bool IsActive() const override
			{
				return m_IsActive && IsInstalled();
			}
			void SetActive(bool value) override
			{
				m_IsActive = value;
			}
			bool IsInstalled() const override
			{
				return m_FileTree.HasChildren();
			}

			bool HasColor() const override
			{
				return m_Color.IsOk();
			}
			KxColor GetColor() const override
			{
				return m_Color;
			}
			void SetColor(const KxColor& color)override 
			{
				m_Color = color;
			}
			KImageEnum BasicGameMod::GetIcon() const
			{
				return KIMG_NONE;
			}

			bool IsLinkedMod() const override
			{
				return !m_LinkLocation.IsEmpty();
			}
			void LinkLocation(const wxString& path) override
			{
				m_LinkLocation = path;
			}
			void UnlinkLocation() override
			{
				m_LinkLocation.clear();
			}

			intptr_t GetPriority() const override;
			intptr_t GetOrderIndex() const override;
			wxString GetModFilesDir() const override;
	};
}