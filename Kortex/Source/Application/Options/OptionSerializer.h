#pragma once
#include "stdafx.h"
class KxDataViewCtrl;
class KxSplitterWindow;
class wxTopLevelWindow;

namespace KxDataView2
{
	class View;
}

namespace Kortex
{
	class IAppOption;
}

namespace Kortex::Application::OptionSerializer
{
	enum class SerializationMode
	{
		Save,
		Load
	};
}

namespace Kortex::Application::OptionSerializer
{
	class UILayout
	{
		protected:
			using SerializationMode = OptionSerializer::SerializationMode;

		protected:
			static void DataViewLayout(IAppOption& option, SerializationMode mode, KxDataViewCtrl* dataView);
			static void DataView2Layout(IAppOption& option, SerializationMode mode, KxDataView2::View* dataView);
			static void SplitterLayout(IAppOption& option, SerializationMode mode, KxSplitterWindow* window);
			static void WindowSize(IAppOption& option, SerializationMode mode, wxTopLevelWindow* window);
	};
}
