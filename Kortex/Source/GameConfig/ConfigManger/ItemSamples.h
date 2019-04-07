#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ItemValue.h"
class KxXMLNode;

namespace Kortex::GameConfig
{
	class Item;

	enum class CompareResult
	{
		LessThan = 1,
		Equal = 2,
		GreaterThan = 3
	};
	CompareResult CompareStrings(const wxString& v1, const wxString& v2, SortOptionsValue sortOptions);
}

namespace Kortex::GameConfig
{
	enum class SamplingFunctionID: uint32_t
	{
		None = 0,
		FindFiles,
		GetAvailableTranslations,
		GetStartupWorkspaces,
		GetVideoAdapters,
		GetVideoModes,
		GetVirtualKeys,
	};
	struct SamplingFunctionDef: public KxIndexedEnum::Definition<SamplingFunctionDef, SamplingFunctionID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{SamplingFunctionID::None, wxS("None")},
			{SamplingFunctionID::FindFiles, wxS("FindFiles")},
			{SamplingFunctionID::GetAvailableTranslations, wxS("GetAvailableTranslations")},
			{SamplingFunctionID::GetStartupWorkspaces, wxS("GetStartupWorkspaces")},
			{SamplingFunctionID::GetVideoAdapters, wxS("GetVideoAdapters")},
			{SamplingFunctionID::GetVideoModes, wxS("GetVideoModes")},
			{SamplingFunctionID::GetVirtualKeys, wxS("GetVirtualKeys")},
		};
	};
	using SamplingFunctionValue = KxIndexedEnum::Value<SamplingFunctionDef, SamplingFunctionID::None>;
}

namespace Kortex::GameConfig
{
	class SampleValue
	{
		public:
			using Vector = std::vector<SampleValue>;

		private:
			ItemValue m_Value;
			wxString m_Label;

		public:
			SampleValue() = default;
			SampleValue(SampleValue&& other)
			{
				*this = std::move(other);
			}
			SampleValue(const SampleValue& other)
			{
				*this = other;
			}

			template<class T> SampleValue(const T& value)
				:m_Value(value)
			{
			}

		public:
			const ItemValue& GetValue() const
			{
				return m_Value;
			}
			ItemValue& GetValue()
			{
				return m_Value;
			}

			bool HasLabel() const
			{
				return !m_Label.IsEmpty();
			}
			wxString GetLabel() const
			{
				return m_Label.IsEmpty() ? m_Value.As<wxString>() : m_Label;
			}
			void SetLabel(const wxString& label)
			{
				m_Label = label;
			}
	
		public:
			SampleValue& operator=(SampleValue&& other)
			{
				m_Value = std::move(other.m_Value);
				m_Label = std::move(other.m_Label);

				return *this;
			}
			SampleValue& operator=(const SampleValue& other)
			{
				m_Value = other.m_Value;
				m_Label = other.m_Label;

				return *this;
			}
	};
}

namespace Kortex::GameConfig
{
	class ISamplingFunction;

	class ItemSamples
	{
		friend class ISamplingFunction;

		private:
			Item& m_Item;
			SamplesSourceValue m_SourceType;
			SortOrderValue m_SortOrder;
			SortOptionsValue m_SortOptions;
			SamplingFunctionValue m_SampligFunction;

			SampleValue::Vector m_Values;
			wxAny m_MinValue;
			wxAny m_MaxValue;
			wxAny m_Step;

		private:
			size_t LoadImmediateItems(const KxXMLNode& rootNode);
			void SortImmediateItems();
			void GenerateItems(const ItemValue::Vector& arguments);
			template<class T> SortOrderValue LoadRange(T min, T max, T step)
			{
				if constexpr(std::is_signed_v<T>)
				{
					m_Values.reserve(std::abs(max - min));
				}

				m_MinValue = min;
				m_MaxValue = max;
				m_Step = step;

				for (T i = min; i <= max; i += step)
				{
					SampleValue& sample = m_Values.emplace_back();
					sample.GetValue().Assign(i);
				}
				return step >= 0 ? SortOrderID::Ascending : SortOrderID::Descending;
			}

			SampleValue::Vector& GetSampleValues()
			{
				return m_Values;
			}
			template<class TItems, class TFunctor> static void DoForEachItem(TItems&& items, TFunctor&& func)
			{
				for (auto&& item: items)
				{
					func(item);
				}
			}

		public:
			ItemSamples(Item& item, const KxXMLNode& samplesNode = {});

		public:
			void Load(const KxXMLNode& samplesNode);
			
			bool IsEmpty() const
			{
				return m_Values.empty();
			}
			size_t GetCount() const
			{
				return m_Values.size();
			}
			Item& GetItem() const
			{
				return m_Item;
			}

			SamplesSourceValue GetSourceType() const
			{
				return m_SourceType;
			}
			SortOrderValue GetSortOrder() const
			{
				return m_SortOrder;
			}
			SortOptionsValue GetSortOptions() const
			{
				return m_SortOptions;
			}
			SamplingFunctionValue GetSamplingFunction() const
			{
				return m_SampligFunction;
			}

			bool HasStep() const;
			bool HasBoundValues() const;
			template<class T> void GetBoundValues(T& min, T& max) const
			{
				static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

				if (m_MinValue.IsNull() || !m_MinValue.GetAs(&min))
				{
					min = std::numeric_limits<T>::lowest();
				}
				if (m_MaxValue.IsNull() || !m_MaxValue.GetAs(&max))
				{
					max = std::numeric_limits<T>::max();
				}
			}

			template<class TFunctor> void ForEachSample(TFunctor&& func) const
			{
				DoForEachItem(m_Values, func);
			}
			template<class TFunctor> void ForEachSample(TFunctor&& func)
			{
				DoForEachItem(m_Values, func);
			}
			
			const SampleValue* GetSampleByIndex(size_t index) const
			{
				if (index < m_Values.size())
				{
					return &m_Values[index];
				}
				return nullptr;
			}
			const SampleValue* FindSampleByValue(const ItemValue& value, size_t* index = nullptr) const;
			const SampleValue* FindSampleByLabel(const wxString& label, size_t* index = nullptr) const;
	};
}
