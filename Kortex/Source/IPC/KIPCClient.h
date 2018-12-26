#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "IPC/KIPCRequest.h"
#include <KxFramework/KxSingleton.h>
class KIPCConnection;

class KIPCClient: public wxClient, public KxSingletonPtr<KIPCClient>
{
	friend class KIPCConnection;

	public:
		static wxString GetServerFileName();
		static bool RunServerAndConnect(KIPCClient** clientInstance);

	private:
		KIPCConnection* m_Connection = nullptr;

	private:
		bool CreateConnection();
		virtual wxConnectionBase* OnMakeConnection() override;
		virtual void OnDisconnect();

		void OnAcceptRequest(const KIPCRequestNS::VFSStateChanged& config);

	public:
		KIPCClient();
		virtual ~KIPCClient();

	public:
		bool IsConnected() const
		{
			return m_Connection != nullptr;
		}

		bool InitConnection();
		bool Disconnect();
		bool InitVFSService();
		bool UninstallVFSService();

		bool CreateVFS_Mirror(const wxString& source, const wxString& target);
		bool CreateVFS_MultiMirror(const KxStringVector& sources, const wxString& target);
		bool MirrorVFS_ClearList();

		bool CreateVFS_Convergence(const wxString& source, const wxString& writeTarget, const KxStringVector& virtualFolders, bool canDeleteInVirtualFolder);
		bool ConvergenceVFS_ClearVirtualFolders();
		bool ConvergenceVFS_BuildDispatcherIndex();
		bool ConvergenceVFS_SetDispatcherIndex();
		
		bool EnableVFS();
		bool DisableVFS();
};
