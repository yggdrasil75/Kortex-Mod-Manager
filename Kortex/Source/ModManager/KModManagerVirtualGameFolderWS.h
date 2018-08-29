#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>

class KModManagerVirtualGameFolderWS: public KWorkspace, public KxSingletonPtr<KModManagerVirtualGameFolderWS>
{
	private:
		KProgramOptionUI m_OptionsUI;
		KProgramOptionUI m_ModListViewOptions;

		wxBoxSizer* m_MainSizer = NULL;

	public:
		KModManagerVirtualGameFolderWS(KMainWindow* mainWindow);
		virtual ~KModManagerVirtualGameFolderWS();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetNameShort() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_FOLDERS;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		
		virtual bool IsSubWorkspace() const
		{
			return true;
		}
		virtual bool CanReload() const override
		{
			return true;
		}
};
