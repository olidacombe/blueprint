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
  }

#define BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(setter, ...)    \
  [](const juce::var& value, YGNodeRef node) {                               \
      BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(value, setter, node, __VA_ARGS__); \
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

#define BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER(setter)     \
  [](const juce::var& value, YGNodeRef node) {             \
      BP_SET_FLEX_DIMENSION_PROPERTY(value, setter, node); \
  }

#define BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(setter, ...)    \
  [](const juce::var& value, YGNodeRef node) {                          \
      BP_SET_FLEX_DIMENSION_PROPERTY(value, setter, node, __VA_ARGS__); \
  }

#define BP_SET_FLEX_FLOAT_PROPERTY(value, setter, node)    \
{                                                          \
    if (value.isDouble())                                  \
        setter(node, (float) value);                       \
}

#define BP_YOGA_NODE_FLOAT_PROPERTY_SETTER(setter)         \
  [](const juce::var& value, YGNodeRef node) {             \
      BP_SET_FLEX_FLOAT_PROPERTY(value, setter, node);     \
  }

#define BP_YOGA_ENUM_PROPERTY_SETTER(setter, enumMap)                \
  [](const juce::var& value, YGNodeRef node) {                       \
    try {                                                            \
      setter(node, enumMap.at(value));                               \
    } catch(const std::out_of_range& e) { /* TODO log something */ } \
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
      typedef std::function<void(const V&, YGNodeRef)> F;
      std::map<K, std::function<void(const juce::var&, YGNodeRef)>> propertySetters;

      public:
        PropertySetterMap(std::initializer_list<std::pair<const K, F>> init): propertySetters(init) {}
        bool call(const K& key, const V& v, YGNodeRef node) const {
          const auto setter = propertySetters.find(key);
          if(setter == propertySetters.end()) {
            return false;
          }
          setter->second(v, node);
          return true;
        }
    };

    static const PropertySetterMap propertySetters{
        {"direction", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetDirection, ValidDirectionValues)},
        {"flex-direction", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetFlexDirection, ValidFlexDirectionValues)},
        {"justify-content", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetJustifyContent, ValidJustifyValues)},
        {"align-items", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetAlignItems, ValidAlignValues)},
        {"align-content", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetAlignContent, ValidAlignValues)},
        {"align-self", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetAlignSelf, ValidAlignValues)},
        {"position", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetPositionType, ValidPositionTypeValues)},
        {"flex-wrap", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetFlexWrap, ValidFlexWrapValues)},
        {"overflow", BP_YOGA_ENUM_PROPERTY_SETTER(YGNodeStyleSetOverflow, ValidOverflowValues)},
        {"flex", BP_YOGA_NODE_FLOAT_PROPERTY_SETTER(YGNodeStyleSetFlex)},
        {"flex-grow", BP_YOGA_NODE_FLOAT_PROPERTY_SETTER(YGNodeStyleSetFlexGrow)},
        {"flex-shrink", BP_YOGA_NODE_FLOAT_PROPERTY_SETTER(YGNodeStyleSetFlexShrink)},
        {"flex-basis", BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER(YGNodeStyleSetFlexBasis)},
        {"width", BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER(YGNodeStyleSetWidth)},
        {"height", BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER(YGNodeStyleSetHeight)},
        {"min-width", BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER(YGNodeStyleSetMinWidth)},
        {"min-height", BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER(YGNodeStyleSetMinHeight)},
        {"max-width", BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER(YGNodeStyleSetMaxWidth)},
        {"max-height", BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER(YGNodeStyleSetMaxHeight)},
        {"aspect-ratio", BP_YOGA_NODE_FLOAT_PROPERTY_SETTER(YGNodeStyleSetAspectRatio)},

        {"margin", BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeAll)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeLeft), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeLeft)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeTop), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeTop)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeRight), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeRight)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeBottom), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeBottom)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeStart), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeStart)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeEnd), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeEnd)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeHorizontal), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeHorizontal)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeVertical), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeLeft)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeAll), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeAll)},

        {"padding", BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeAll)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeLeft), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeLeft)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeTop), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeTop)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeRight), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeRight)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeBottom), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeBottom)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeStart), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeStart)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeEnd), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeEnd)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeHorizontal), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeHorizontal)},
        {juce::String("padding-") + YGEdgeToString(YGEdgeVertical), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPadding, YGEdgeLeft)},
        {juce::String("margin-") + YGEdgeToString(YGEdgeAll), BP_YOGA_NODE_DIMENSION_PROPERTY_AUTO_SETTER_VARIADIC(YGNodeStyleSetMargin, YGEdgeAll)},
        
        {YGEdgeToString(YGEdgeLeft), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPosition, YGEdgeLeft)},
        {YGEdgeToString(YGEdgeTop), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPosition, YGEdgeTop)},
        {YGEdgeToString(YGEdgeRight), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPosition, YGEdgeRight)},
        {YGEdgeToString(YGEdgeBottom), BP_YOGA_NODE_DIMENSION_PROPERTY_SETTER_VARIADIC(YGNodeStyleSetPosition, YGEdgeBottom)},
    };

    //==============================================================================
    bool ShadowView::setProperty (const juce::Identifier& name, const juce::var& newValue)
    {
        props.set(name, newValue);

        return propertySetters.call(name, newValue, yogaNode);
    }
}
