#include "stdafx.h"
#include "GameID.h"
#include "IGameInstance.h"
#include "Application/SystemApplication.h"

namespace Kortex
{
	bool GameID::TestGameID(const wxString& id) const
	{
		if (!id.IsEmpty())
		{
			// Assume we can check ID if there are at least one shallow instance.
			// Otherwise just check is the string is not empty.
			SystemApplication* sysApp = SystemApplication::GetInstance();
			if (sysApp && !sysApp->GetShallowGameInstances().empty())
			{
				return GetInstanceByID(id) != nullptr;
			}
			return true;
		}
		return false;
	}
	IGameInstance* GameID::GetInstanceByID(const wxString& id) const
	{
		if (!id.IsEmpty())
		{
			IGameInstance* active = IGameInstance::GetActive();
			if (active && active->GetGameID() == id)
			{
				return active;
			}
			return IGameInstance::GetShallowInstance(id);
		}
		return nullptr;
	}

	GameID::GameID(const wxString& id)
		:m_ID(TestGameID(id) ? id : wxEmptyString)
	{
	}
	GameID::GameID(const IGameInstance& instance)
		:m_ID(instance.IsOK() ? instance.GetGameID().m_ID : wxEmptyString)
	{
	}

	bool GameID::IsOK() const
	{
		if (!TestGameID(m_ID))
		{
			const_cast<wxString&>(m_ID).clear();
			return false;
		}
		return true;
	}
	wxString GameID::ToString() const
	{
		return IsOK() ? m_ID : wxEmptyString;
	}
	IGameInstance* GameID::ToGameInstance() const
	{
		return GetInstanceByID(m_ID);
	}

	GameID& GameID::operator=(const wxString& id)
	{
		m_ID = TestGameID(id) ? id : wxEmptyString;
		return *this;
	}
}
