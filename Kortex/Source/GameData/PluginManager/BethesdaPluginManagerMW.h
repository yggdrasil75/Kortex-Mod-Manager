#pragma once
#include "stdafx.h"
#include "BethesdaPluginManager.h"
class KxINI;

namespace Kortex::PluginManager
{
	class BethesdaPluginManagerMW: public KxRTTI::ExtendInterface<BethesdaPluginManagerMW, BethesdaPluginManager>
	{
		KxDecalreIID(BethesdaPluginManagerMW, {0xc7657504, 0x9232, 0x4e79, {0x8f, 0xf7, 0xf9, 0x8d, 0x9, 0x73, 0x92, 0x74}});

		private:
			const wxString m_PluginsListFile;

		private:
			void ReadOrderMW(const KxINI& ini);
			void ReadActiveMW(const KxINI& ini);
			void WriteOrderMW(KxINI& ini) const;
			void WriteActiveMW(KxINI& ini) const;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			void LoadNativeActiveBG() override;
			void LoadNativeOrderBG() override;
			void SaveNativeOrderBG() const override;
			wxString GetMorrowindINI() const;	

		public:
			BethesdaPluginManagerMW();
			virtual ~BethesdaPluginManagerMW();

		public:
			wxString GetPluginRootRelativePath(const wxString& fileName) const override
			{
				return m_PluginsLocation + wxS('\\') + fileName;
			}

			void Save() const override;
			void Load() override;
			void LoadNativeOrder() override;
	};
}
