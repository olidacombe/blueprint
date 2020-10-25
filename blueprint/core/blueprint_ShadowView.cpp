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
      setter(node, enumMap.at(value));                                   \
    } catch(const std::out_of_range& e) { /* TODO log something */ } \
  }

namespace blueprint
{

    namespace
    {
        //==============================================================================
        struct FlexProperties
        {
            // Flex enums
            juce::Identifier direction       = "direction";
            juce::Identifier flexDirection   = "flex-direction";
            juce::Identifier justifyContent  = "justify-content";
            juce::Identifier alignItems      = "align-items";
            juce::Identifier alignContent    = "align-content";
            juce::Identifier alignSelf       = "align-self";
            juce::Identifier position        = "position";
            juce::Identifier flexWrap        = "flex-wrap";
            juce::Identifier overflow        = "overflow";

            // Flex dimensions
            juce::Identifier flex            = "flex";
            juce::Identifier flexGrow        = "flex-grow";
            juce::Identifier flexShrink      = "flex-shrink";
            juce::Identifier flexBasis       = "flex-basis";
            juce::Identifier width           = "width";
            juce::Identifier height          = "height";
            juce::Identifier minWidth        = "min-width";
            juce::Identifier minHeight       = "min-height";
            juce::Identifier maxWidth        = "max-width";
            juce::Identifier maxHeight       = "max-height";
            juce::Identifier aspectRatio     = "aspect-ratio";

            // Margin
            juce::Identifier marginMetaProp   = "margin";
            juce::Identifier marginLeft       = juce::String("margin-") + YGEdgeToString(YGEdgeLeft);
            juce::Identifier marginRight      = juce::String("margin-") + YGEdgeToString(YGEdgeRight);
            juce::Identifier marginTop        = juce::String("margin-") + YGEdgeToString(YGEdgeTop);
            juce::Identifier marginBottom     = juce::String("margin-") + YGEdgeToString(YGEdgeBottom);
            juce::Identifier marginStart      = juce::String("margin-") + YGEdgeToString(YGEdgeStart);
            juce::Identifier marginEnd        = juce::String("margin-") + YGEdgeToString(YGEdgeEnd);
            juce::Identifier marginHorizontal = juce::String("margin-") + YGEdgeToString(YGEdgeHorizontal);
            juce::Identifier marginVertical   = juce::String("margin-") + YGEdgeToString(YGEdgeVertical);

            // Padding
            juce::Identifier paddingMetaProp   = "padding";
            juce::Identifier paddingLeft       = juce::String("padding-") + YGEdgeToString(YGEdgeLeft);
            juce::Identifier paddingRight      = juce::String("padding-") + YGEdgeToString(YGEdgeRight);
            juce::Identifier paddingTop        = juce::String("padding-") + YGEdgeToString(YGEdgeTop);
            juce::Identifier paddingBottom     = juce::String("padding-") + YGEdgeToString(YGEdgeBottom);
            juce::Identifier paddingStart      = juce::String("padding-") + YGEdgeToString(YGEdgeStart);
            juce::Identifier paddingEnd        = juce::String("padding-") + YGEdgeToString(YGEdgeEnd);
            juce::Identifier paddingHorizontal = juce::String("padding-") + YGEdgeToString(YGEdgeHorizontal);
            juce::Identifier paddingVertical   = juce::String("padding-") + YGEdgeToString(YGEdgeVertical);

            //Position
            juce::Identifier positionLeft      = YGEdgeToString(YGEdgeLeft);
            juce::Identifier positionRight     = YGEdgeToString(YGEdgeRight);
            juce::Identifier positionTop       = YGEdgeToString(YGEdgeTop);
            juce::Identifier positionBottom    = YGEdgeToString(YGEdgeBottom);
            //TODO: Any need for other edge values? i.e. start, end ?
        } flexProperties;

        bool isMarginProp(const juce::Identifier& prop)
        {
            static const std::array<juce::Identifier, 9> marginProps =
            {
                flexProperties.marginMetaProp,
                flexProperties.marginLeft,
                flexProperties.marginRight,
                flexProperties.marginTop,
                flexProperties.marginBottom,
                flexProperties.marginStart,
                flexProperties.marginEnd,
                flexProperties.marginHorizontal,
                flexProperties.marginVertical
            };

            return std::find(marginProps.cbegin(), marginProps.cend(), prop) != marginProps.cend();
        }

        bool isPaddingProp(const juce::Identifier& prop)
        {
            static std::array<juce::Identifier, 9> paddingProps =
            {
                flexProperties.paddingMetaProp,
                flexProperties.paddingLeft,
                flexProperties.paddingRight,
                flexProperties.paddingTop,
                flexProperties.paddingBottom,
                flexProperties.paddingStart,
                flexProperties.paddingEnd,
                flexProperties.paddingHorizontal,
                flexProperties.paddingVertical
            };

            return std::find(paddingProps.cbegin(), paddingProps.cend(), prop) != paddingProps.cend();
        }

        bool isPositionProp(const juce::Identifier& prop)
        {
            static std::array<juce::Identifier, 4> positionProps =
            {
                flexProperties.positionLeft,
                flexProperties.positionRight,
                flexProperties.positionTop,
                flexProperties.positionBottom,
            };

            return std::find(positionProps.cbegin(), positionProps.cend(), prop) != positionProps.cend();
        }

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

        std::map<juce::String, YGEdge> ValidEdgeValues {
            { YGEdgeToString(YGEdgeLeft), YGEdgeLeft },
            { YGEdgeToString(YGEdgeTop), YGEdgeTop },
            { YGEdgeToString(YGEdgeRight), YGEdgeRight },
            { YGEdgeToString(YGEdgeBottom), YGEdgeBottom },
            { YGEdgeToString(YGEdgeStart), YGEdgeStart },
            { YGEdgeToString(YGEdgeEnd), YGEdgeEnd },
            { YGEdgeToString(YGEdgeHorizontal), YGEdgeHorizontal },
            { YGEdgeToString(YGEdgeVertical), YGEdgeVertical },
            { YGEdgeToString(YGEdgeAll), YGEdgeAll },
        };

        //==============================================================================
        template<typename T>
        bool validateFlexProperty (juce::String value, std::map<juce::String, T> validValues)
        {
            return std::any_of(validValues.cbegin(), validValues.cend(), [=] (const auto &pair)
            {
                return value.equalsIgnoreCase(pair.first);
            });
        }
    }

    //==============================================================================
    bool ShadowView::isLayoutProperty(const juce::Identifier& property)
    {
        static const std::array<juce::Identifier, 21> properties =
        {
            flexProperties.direction,
            flexProperties.flexDirection,
            flexProperties.justifyContent,
            flexProperties.alignItems,
            flexProperties.alignContent,
            flexProperties.alignSelf,
            flexProperties.position,
            flexProperties.flexWrap,
            flexProperties.overflow,
            flexProperties.flex,
            flexProperties.flexGrow,
            flexProperties.flexShrink,
            flexProperties.flexBasis,
            flexProperties.width,
            flexProperties.height,
            flexProperties.minWidth,
            flexProperties.minHeight,
            flexProperties.minWidth,
            flexProperties.maxWidth,
            flexProperties.maxHeight,
            flexProperties.aspectRatio
        };

        if (std::find(properties.cbegin(), properties.cend(), property) != properties.cend())
            return true;
        else if (isMarginProp(property))
            return true;
        else if (isPaddingProp(property))
            return true;
        else if (isPositionProp(property))
            return true;
        else
            return false;
    }

    class PropertySetterMap {
      typedef juce::Identifier K;
      typedef juce::var V;
      typedef std::function<void(const V&, YGNodeRef)> F;
      typedef std::initializer_list<std::pair<const juce::String, F>> PrefixedEdgeSetterInitList;
      std::map<juce::Identifier, std::function<void(const juce::var&, YGNodeRef)>> propertySetters;

      public:
        PropertySetterMap(std::initializer_list<std::pair<const K, F>> init): propertySetters(init) {}
        // TODO cut down indirection?
        const F& at(const K& k) const {
          return propertySetters.at(k);
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

        try {
          (propertySetters.at(name))(newValue, yogaNode);
          return true;
        } catch(const std::out_of_range& e) {}

        //==============================================================================
        // Margin
        if (isMarginProp(name))
        {
            if (name == flexProperties.marginMetaProp)
                BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(newValue, YGNodeStyleSetMargin, yogaNode, YGEdgeAll)
            else
                BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(newValue, YGNodeStyleSetMargin, yogaNode, ValidEdgeValues[name.toString().replace("margin-", "")])

           return true;
        }

        //==============================================================================
        // Padding
        if (isPaddingProp(name))
        {
            if (name == flexProperties.paddingMetaProp)
                BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetPadding, yogaNode, YGEdgeAll)
            else
                BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetPadding, yogaNode, ValidEdgeValues[name.toString().replace("padding-", "")])

            return true;
        }

        //==============================================================================
        // Position
        if (isPositionProp(name))
        {
            BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetPosition, yogaNode, ValidEdgeValues[name.toString()]);
            return true;
        }
        return false;
    }
}
