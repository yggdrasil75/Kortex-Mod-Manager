#include "stdafx.h"
#include "DefaultNetworkManager.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Events.hpp>
#include "Network/ModNetwork/Nexus.h"
#include "Network/ModNetwork/LoversLab.h"
#include "Network/ModNetwork/TESALL.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxMenu.h>

namespace
{
	bool IsDefaultModNetworkAuthenticated()
	{
		using namespace Kortex;

		const IModNetwork* modNetwork= INetworkManager::GetInstance()->GetDefaultModNetwork();
		const ModNetworkAuth* auth = nullptr;

		return modNetwork && modNetwork->TryGetComponent(auth) && auth->IsAuthenticated();
	};
	template<class TModNetwork, class TContainer> TModNetwork& AddModNetwork(TContainer&& container)
	{
		return static_cast<TModNetwork&>(*container.emplace_back(Kortex::IModNetwork::Create<TModNetwork>()));
	}
}

namespace Kortex::NetworkManager
{
	void DefaultNetworkManager::OnInit()
	{
		using namespace NetworkManager;
		using namespace Application;

		// Init sources
		m_ModNetworks.reserve(3);
		AddModNetwork<NexusModNetwork>(m_ModNetworks);
		AddModNetwork<LoversLabModNetwork>(m_ModNetworks);
		AddModNetwork<TESALLModNetwork>(m_ModNetworks);

		// Load default source
		if (IModNetwork* modNetwork = GetModNetworkByName(GetAInstanceOption(OName::ModSource).GetAttribute(OName::Default)))
		{
			m_DefaultModNetwork = modNetwork;
		}
		KxFile(GetCacheFolder()).CreateFolder();
	}
	void DefaultNetworkManager::OnExit()
	{
		using namespace Application;

		const IModNetwork* modNetwork = GetDefaultModNetwork();
		GetAInstanceOption(OName::ModSource).SetAttribute(OName::Default, modNetwork ? modNetwork->GetName() : wxEmptyString);
	}
	void DefaultNetworkManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}

	void DefaultNetworkManager::ValidateAuth()
	{
		for (auto& modNetwork: m_ModNetworks)
		{
			if (auto auth = modNetwork->TryGetComponent<ModNetworkAuth>())
			{
				auth->ValidateAuth();
			}
		}
		AdjustDefaultModNetwork();
	}
	bool DefaultNetworkManager::AdjustDefaultModNetwork()
	{
		if (!IsDefaultModNetworkAuthenticated())
		{
			m_DefaultModNetwork = nullptr;
			for (auto& modNetwork: m_ModNetworks)
			{
				const ModNetworkAuth* auth = nullptr;
				if (modNetwork->TryGetComponent(auth) && auth->IsAuthenticated())
				{
					m_DefaultModNetwork = modNetwork.get();
					return true;
				}
			}
			return false;
		}
		return true;
	}

	void DefaultNetworkManager::OnSetToolBarButton(KxAuiToolBarItem* button)
	{
		m_LoginButton = button;

		ValidateAuth();
		UpdateButton();
		CreateMenu();
	}
	void DefaultNetworkManager::UpdateButton()
	{
		const ModNetworkAuth* auth = nullptr;
		if (m_DefaultModNetwork && m_DefaultModNetwork->TryGetComponent(auth) && auth->IsAuthenticated())
		{
			m_LoginButton->SetLabel(KTr("NetworkManager.SignedIn") + ": " + m_DefaultModNetwork->GetName());
			m_LoginButton->SetBitmap(KGetBitmap(m_DefaultModNetwork->GetIcon()));
		}
		else
		{
			m_LoginButton->SetLabel(KTr("NetworkManager.NotSignedIn"));
			m_LoginButton->SetBitmap(KGetBitmap(IModNetwork::GetGenericIcon()));
		}

		m_LoginButton->GetToolBar()->Realize();
		m_LoginButton->GetToolBar()->Refresh();
		KMainWindow::GetInstance()->Layout();
	}
	void DefaultNetworkManager::CreateMenu()
	{
		KxMenu::EndMenu();
		m_Menu = new KxMenu();
		m_LoginButton->AssignDropdownMenu(m_Menu);

		for (const auto& modNetwork: m_ModNetworks)
		{
			if (KxMenu* subMenu = new KxMenu(); true)
			{
				KxMenuItem* rootItem = m_Menu->Add(subMenu, modNetwork->GetName());
				rootItem->SetBitmap(KGetBitmap(modNetwork->GetIcon()));

				const ModNetworkAuth* authenticable = modNetwork->TryGetComponent<ModNetworkAuth>();
				const ModNetworkRepository* repository = modNetwork->TryGetComponent<ModNetworkRepository>();

				// Add default source toggle
				{
					KxMenuItem* item = subMenu->Add(new KxMenuItem(wxEmptyString, wxEmptyString, wxITEM_CHECK));
					item->SetClientData(modNetwork.get());
					item->Enable(false);

					if (authenticable)
					{
						if (m_DefaultModNetwork == modNetwork.get())
						{
							item->Check();
							item->SetItemLabel(KTr("NetworkManager.ModNetwork.Default"));
						}
						else if (!authenticable->IsAuthenticated())
						{
							wxString label = KxString::Format("%1 (%2)", KTr("NetworkManager.ModNetwork.MakeDefault"), KTr("NetworkManager.NotSignedIn"));
							item->SetItemLabel(KxString::MakeCapitalized(label));
						}
						else
						{
							item->Enable();
							item->SetItemLabel(KTr("NetworkManager.ModNetwork.MakeDefault"));
							item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSelectDefaultModSource, this);
						}
					}
					else
					{
						item->SetItemLabel(KTr("NetworkManager.ModNetwork.MakeDefault"));
					}
				}

				// Add sign-in/sign-out items.
				if (authenticable)
				{
					wxString label;
					if (bool isAuth = authenticable->IsAuthenticated())
					{
						if (auto credentials = authenticable->LoadCredentials())
						{
							label = KxString::Format(wxS("%1: %2"), KTr("NetworkManager.SignOut"), credentials->UserID);
						}
						else
						{
							label = KTr("NetworkManager.SignOut");
						}
					}
					else
					{
						label = KTr("NetworkManager.SignIn");
					}

					KxMenuItem* item = subMenu->Add(new KxMenuItem(label));
					item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSignInOut, this);
					item->SetBitmap(authenticable->GetUserPicture());
					item->SetClientData(modNetwork.get());
				}

				// Add limits information display
				if (repository)
				{
					if (ModRepositoryLimits limits = repository->GetRequestLimits(); limits.IsOK() && limits.HasLimits())
					{
						wxString label;
						auto AddSeparator = [&label]()
						{
							if (!label.IsEmpty())
							{
								label += wxS("; ");
							}
						};

						if (limits.HasHourlyLimit())
						{
							label += KTrf(wxS("NetworkManager.QueryLimits.Hourly"), limits.GetHourlyRemaining(), limits.GetHourlyLimit());
						}
						if (limits.HasDailyLimit())
						{
							AddSeparator();
							label += KTrf(wxS("NetworkManager.QueryLimits.Daily"), limits.GetDailyRemaining(), limits.GetDailyLimit());
						}
						label = KxString::Format(wxS("%1: [%2]"), KTr(wxS("NetworkManager.QueryLimits")), label);

						KxMenuItem* item = subMenu->Add(new KxMenuItem(label));
						item->SetBitmap(KGetBitmap(limits.AnyLimitDepleted() ? KIMG_EXCLAMATION : KIMG_TICK_CIRCLE_FRAME));
						item->SetClientData(modNetwork.get());
						item->Enable(false);
					}
				}
			}
		}
	}
	void DefaultNetworkManager::QueueUIUpdate()
	{
		IEvent::CallAfter([this]()
		{
			CreateMenu();
			UpdateButton();
		});
	}

	void DefaultNetworkManager::OnSignInOut(KxMenuEvent& event)
	{
		IModNetwork* modNetwork = static_cast<IModNetwork*>(event.GetItem()->GetClientData());
		if (auto auth = modNetwork->TryGetComponent<ModNetworkAuth>(); auth && auth->IsAuthenticated())
		{
			KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTrf("NetworkManager.SignOutMessage", modNetwork->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				auth->SignOut();
				AdjustDefaultModNetwork();
				QueueUIUpdate();
			}
		}
		else
		{
			// Additional call to "IModSource::IsAuthenticated' to make sure that modNetwork is ready
			// as authentication process can be async.
			if (auth->Authenticate() && auth->IsAuthenticated() && !IsDefaultModNetworkAuthenticated())
			{
				m_DefaultModNetwork = modNetwork;
			}
			QueueUIUpdate();
		}
	}
	void DefaultNetworkManager::OnSelectDefaultModSource(KxMenuEvent& event)
	{
		IModNetwork* modNetwork = static_cast<IModNetwork*>(event.GetItem()->GetClientData());
		m_DefaultModNetwork = modNetwork;

		UpdateButton();
		GetAInstanceOption().SetAttribute("DefaultProvider", modNetwork->GetName());
	}
	void DefaultNetworkManager::OnToolBarButton(KxAuiToolBarEvent& event)
	{
		m_LoginButton->ShowDropdownMenuLeftAlign();
	}

	void DefaultNetworkManager::OnAuthStateChanged()
	{
		AdjustDefaultModNetwork();

		CreateMenu();
		UpdateButton();
	}
	wxString DefaultNetworkManager::GetCacheFolder() const
	{
		return IApplication::GetInstance()->GetUserSettingsFolder() + wxS("\\WebCache");
	}

	IModNetwork* DefaultNetworkManager::GetDefaultModNetwork() const
	{
		return m_DefaultModNetwork;
	}
	IModNetwork* DefaultNetworkManager::GetModNetworkByName(const wxString& name) const
	{
		auto FindModNetwork = [this](const wxString& name) -> IModNetwork*
		{
			for (const auto& modNetwork: m_ModNetworks)
			{
				if (modNetwork->GetName() == name)
				{
					return modNetwork.get();
				}
			}
			return nullptr;
		};

		if (KxComparator::IsEqual(name, wxS("Nexus"), true) || KxComparator::IsEqual(name, wxS("NexusMods"), true))
		{
			return NexusModNetwork::GetInstance();
		}
		else if (KxComparator::IsEqual(name, wxS("TESALL"), true) || KxComparator::IsEqual(name, wxS("TESALL.RU"), true))
		{
			return TESALLModNetwork::GetInstance();
		}
		return FindModNetwork(name);
	}
}

namespace Kortex::NetworkManager
{
	void Config::OnLoadInstance(IGameInstance& profile, const KxXMLNode& node)
	{
		m_NexusID = node.GetFirstChildElement("NexusID").GetValue();
		m_SteamID = node.GetFirstChildElement("SteamID").GetValueInt(m_SteamID);
	}
	wxString Config::GetNexusID() const
	{
		return KVarExp(m_NexusID);
	}
}
