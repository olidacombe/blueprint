/*
  ==============================================================================

    blueprint_ShadowView.cpp
    Created: 17 Apr 2019 10:07:17am

  ==============================================================================
*/

#define BP_SET_YGVALUE(ygvalue, setter, ...)            \
switch (ygvalue.unit)                                   \
{                                                       \
    case YGUnitAuto:                                    \
    case YGUnitUndefined:                               \
        setter(__VA_ARGS__, YGUndefined);               \
        break;                                          \
    case YGUnitPoint:                                   \
        setter(__VA_ARGS__, ygvalue.value);             \
        break;                                          \
    case YGUnitPercent:                                 \
        setter##Percent(__VA_ARGS__, ygvalue.value);    \
        break;                                          \
}

#define BP_SET_YGVALUE_AUTO(ygvalue, setter, ...)       \
switch (ygvalue.unit)                                   \
{                                                       \
    case YGUnitAuto:                                    \
        setter##Auto(__VA_ARGS__);                      \
        break;                                          \
    case YGUnitUndefined:                               \
        setter(__VA_ARGS__, YGUndefined);               \
        break;                                          \
    case YGUnitPoint:                                   \
        setter(__VA_ARGS__, ygvalue.value);             \
        break;                                          \
    case YGUnitPercent:                                 \
        setter##Percent(__VA_ARGS__, ygvalue.value);    \
        break;                                          \
}

#define BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(value, setter, ...)                     \
{                                                                                   \
    YGValue ygval = { 0.0f, YGUnitUndefined };                                      \
                                                                                    \
    if (value.isDouble())                                                           \
        ygval = { (float) value, YGUnitPoint };                                     \
    else if (value.isString() && value.toString() == "auto")                        \
        ygval = { 0.0f, YGUnitAuto };                                               \
    else if (value.isString() && value.toString().trim().contains("%"))             \
    {                                                                               \
        juce::String strVal = value.toString().retainCharacters("-1234567890.");    \
        ygval = { strVal.getFloatValue(), YGUnitPercent };                          \
    }                                                                               \
                                                                                    \
    BP_SET_YGVALUE_AUTO(ygval, setter, __VA_ARGS__);                                \
}

#define BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER(setter)     \
  [](const juce::var& value, YGNodeRef node) {                  \
      BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(value, setter, node); \
      return true; \
  }

#define BP_SET_FLEX_DIMENSION_PROPERTY(value, setter, ...)                          \
{                                                                                   \
    YGValue ygval = { 0.0f, YGUnitUndefined };                                      \
                                                                                    \
    if (value.isDouble())                                                           \
        ygval = { (float) value, YGUnitPoint };                                     \
    else if (value.isString() && value.toString().trim().contains("%"))             \
    {                                                                               \
        juce::String strVal = value.toString().retainCharacters("-1234567890.");    \
        ygval = { strVal.getFloatValue(), YGUnitPercent };                          \
    }                                                                               \
                                                                                    \
    BP_SET_YGVALUE(ygval, setter, __VA_ARGS__);                                     \
}


#define BP_SET_FLEX_FLOAT_PROPERTY(value, setter, node)    \
{                                                          \
    if (value.isDouble())                                  \
        setter(node, (float) value);                       \
}

namespace blueprint
{

    namespace
    {
        //==============================================================================
        std::map<juce::String, YGDirection> ValidDirectionValues {
            { YGDirectionToString(YGDirectionInherit), YGDirectionInherit },
            { YGDirectionToString(YGDirectionLTR), YGDirectionLTR },
            { YGDirectionToString(YGDirectionRTL), YGDirectionRTL },
        };

        std::map<juce::String, YGFlexDirection> ValidFlexDirectionValues {
            { YGFlexDirectionToString(YGFlexDirectionColumn), YGFlexDirectionColumn },
            { YGFlexDirectionToString(YGFlexDirectionColumnReverse), YGFlexDirectionColumnReverse },
            { YGFlexDirectionToString(YGFlexDirectionRow), YGFlexDirectionRow },
            { YGFlexDirectionToString(YGFlexDirectionRowReverse), YGFlexDirectionRowReverse },
        };

        std::map<juce::String, YGJustify> ValidJustifyValues {
            { YGJustifyToString(YGJustifyFlexStart), YGJustifyFlexStart },
            { YGJustifyToString(YGJustifyCenter), YGJustifyCenter },
            { YGJustifyToString(YGJustifyFlexEnd), YGJustifyFlexEnd },
            { YGJustifyToString(YGJustifySpaceBetween), YGJustifySpaceBetween },
            { YGJustifyToString(YGJustifySpaceAround), YGJustifySpaceAround },
        };

        std::map<juce::String, YGAlign> ValidAlignValues {
            { YGAlignToString(YGAlignAuto), YGAlignAuto },
            { YGAlignToString(YGAlignFlexStart), YGAlignFlexStart },
            { YGAlignToString(YGAlignCenter), YGAlignCenter },
            { YGAlignToString(YGAlignFlexEnd), YGAlignFlexEnd },
            { YGAlignToString(YGAlignStretch), YGAlignStretch },
            { YGAlignToString(YGAlignBaseline), YGAlignBaseline },
            { YGAlignToString(YGAlignSpaceBetween), YGAlignSpaceBetween },
            { YGAlignToString(YGAlignSpaceAround), YGAlignSpaceAround },
        };

        std::map<juce::String, YGPositionType> ValidPositionTypeValues {
            { YGPositionTypeToString(YGPositionTypeRelative), YGPositionTypeRelative },
            { YGPositionTypeToString(YGPositionTypeAbsolute), YGPositionTypeAbsolute },
        };

        std::map<juce::String, YGWrap> ValidFlexWrapValues {
            { YGWrapToString(YGWrapNoWrap), YGWrapNoWrap },
            { YGWrapToString(YGWrapWrap), YGWrapWrap },
            { YGWrapToString(YGWrapWrapReverse), YGWrapWrapReverse },
        };

        std::map<juce::String, YGOverflow> ValidOverflowValues {
            { YGOverflowToString(YGOverflowVisible), YGOverflowVisible },
            { YGOverflowToString(YGOverflowHidden), YGOverflowHidden },
            { YGOverflowToString(YGOverflowScroll), YGOverflowScroll },
        };

    }

    //==============================================================================
    class PropertySetterMap {
      typedef juce::Identifier K;
      typedef juce::var V;
      typedef std::function<bool(const V&, YGNodeRef)> F;
      std::map<K, F> propertySetters;

      public:
        PropertySetterMap(std::initializer_list<std::pair<const K, F>> init): propertySetters(init) {}
        bool call(const K& key, const V& v, YGNodeRef node) const {
          const auto setter = propertySetters.find(key);
          if(setter == propertySetters.end()) {
            return false;
          }
          return setter->second(v, node);
        }
    };

    static const PropertySetterMap propertySetters{
        {"direction", getYogaNodeEnumSetter(YGNodeStyleSetDirection, ValidDirectionValues)},
        {"flex-direction", getYogaNodeEnumSetter(YGNodeStyleSetFlexDirection, ValidFlexDirectionValues)},
        {"justify-content", getYogaNodeEnumSetter(YGNodeStyleSetJustifyContent, ValidJustifyValues)},
        {"align-items", getYogaNodeEnumSetter(YGNodeStyleSetAlignItems, ValidAlignValues)},
        {"align-content", getYogaNodeEnumSetter(YGNodeStyleSetAlignContent, ValidAlignValues)},
        {"align-self", getYogaNodeEnumSetter(YGNodeStyleSetAlignSelf, ValidAlignValues)},
        {"position", getYogaNodeEnumSetter(YGNodeStyleSetPositionType, ValidPositionTypeValues)},
        {"flex-wrap", getYogaNodeEnumSetter(YGNodeStyleSetFlexWrap, ValidFlexWrapValues)},
        {"overflow", getYogaNodeEnumSetter(YGNodeStyleSetOverflow, ValidOverflowValues)},
        {"flex", getYogaNodeFloatSetter(YGNodeStyleSetFlex)},
        {"flex-grow", getYogaNodeFloatSetter(YGNodeStyleSetFlexGrow)},
        {"flex-shrink", getYogaNodeFloatSetter(YGNodeStyleSetFlexShrink)},
        {"flex-basis", BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER(YGNodeStyleSetFlexBasis)},
        {"width", getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetWidth))},
        {"height", BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER(YGNodeStyleSetHeight)},
        {"min-width", getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetMinWidth))},
        {"min-height", getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetMinHeight))},
        {"max-width", getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetMaxWidth))},
        {"max-height", getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetMaxHeight))},
        {"aspect-ratio", getYogaNodeFloatSetter(YGNodeStyleSetAspectRatio)},

        {"margin", getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeAll)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeLeft), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeLeft)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeTop), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeTop)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeRight), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeRight)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeBottom), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeBottom)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeStart), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeStart)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeEnd), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeEnd)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeHorizontal), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeHorizontal)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeVertical), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeLeft)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeAll), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeAll)},

        {"padding", getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeAll)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeLeft), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeLeft)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeTop), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeTop)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeRight), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeRight)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeBottom), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeBottom)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeStart), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeStart)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeEnd), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeEnd)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeHorizontal), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeHorizontal)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeVertical), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPadding), YGEdgeLeft)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeAll), getYogaNodeDimensionAutoSetter(BP_SPREAD_SETTER_AUTO(YGNodeStyleSetMargin), YGEdgeAll)},
        
        {YGEdgeToString(YGEdgeLeft), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPosition), YGEdgeLeft)},
        {YGEdgeToString(YGEdgeTop), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPosition), YGEdgeTop)},
        {YGEdgeToString(YGEdgeRight), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPosition), YGEdgeRight)},
        {YGEdgeToString(YGEdgeBottom), getYogaNodeDimensionSetter(BP_SPREAD_SETTER_PERCENT(YGNodeStyleSetPosition), YGEdgeBottom)},
    };

    //==============================================================================
    bool ShadowView::setProperty (const juce::Identifier& name, const juce::var& newValue)
    {
        props.set(name, newValue);

        return propertySetters.call(name, newValue, yogaNode);
    }
}
