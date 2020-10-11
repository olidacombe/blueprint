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
        validationMapKey strVal = value.toString().retainCharacters("-1234567890.");    \
        ygval = { strVal.getFloatValue(), YGUnitPercent };                          \
    }                                                                               \
                                                                                    \
    BP_SET_YGVALUE_AUTO(ygval, setter, __VA_ARGS__);                                \
}

#define BP_SET_FLEX_DIMENSION_PROPERTY(value, setter, ...)                          \
{                                                                                   \
    YGValue ygval = { 0.0f, YGUnitUndefined };                                      \
                                                                                    \
    if (value.isDouble())                                                           \
        ygval = { (float) value, YGUnitPoint };                                     \
    else if (value.isString() && value.toString().trim().contains("%"))             \
    {                                                                               \
        validationMapKey strVal = value.toString().retainCharacters("-1234567890.");    \
        ygval = { strVal.getFloatValue(), YGUnitPercent };                          \
    }                                                                               \
                                                                                    \
    BP_SET_YGVALUE(ygval, setter, __VA_ARGS__);                                     \
}

#define BP_SET_FLEX_FLOAT_PROPERTY(value, setter, node) \
{                                                       \
    if (value.isDouble())                               \
        setter(node, (float) value);                    \
}

namespace blueprint
{

    namespace
    {

        template <typename T>
        T toCamel(const T& i, bool pascal=false) {return pascal?i:i;}
        
        template <>
        std::string toCamel<std::string>(const std::string &i, bool pascal)
        {
            std::string o;
            o.reserve(i.length());
            auto it = i.cbegin();
            bool makeUpper = pascal;
            o += static_cast<char>(makeUpper?toupper(*it++):tolower(*it++));
            makeUpper=false;
            for (; it != i.cend(); it++)
            {
                if (*it == '-' || *it == '_')
                    makeUpper = true;
                else if (makeUpper)
                {
                    o += static_cast<char>(toupper(*it));
                    makeUpper = false;
                }
                else
                    o += *it;
            }
            return o;
        }

        template <>
        juce::String toCamel<juce::String>(const juce::String &i, bool pascal) {
          std::string si = i.toStdString();
          return toCamel(si, pascal);
        }

        typedef juce::String validationMapKey;
        template <typename T>
        const std::map<validationMapKey, T> makeValidationMap(std::initializer_list<std::pair<const validationMapKey, T>> init)
        {
            std::map<validationMapKey, T> map(init);
            for(const auto &[k, v] : map) {
                map[toCamel(k)] = v;
            }
            return std::move(map);
        }

        //==============================================================================
        auto ValidDirectionValues = makeValidationMap<YGDirection>({
            {YGDirectionToString(YGDirectionInherit), YGDirectionInherit},
            {YGDirectionToString(YGDirectionLTR), YGDirectionLTR},
            {YGDirectionToString(YGDirectionRTL), YGDirectionRTL},
        });

        auto ValidFlexDirectionValues = makeValidationMap<YGFlexDirection>({
            {YGFlexDirectionToString(YGFlexDirectionColumn), YGFlexDirectionColumn},
            {YGFlexDirectionToString(YGFlexDirectionColumnReverse), YGFlexDirectionColumnReverse},
            {YGFlexDirectionToString(YGFlexDirectionRow), YGFlexDirectionRow},
            {YGFlexDirectionToString(YGFlexDirectionRowReverse), YGFlexDirectionRowReverse},
        });

        auto ValidJustifyValues = makeValidationMap<YGJustify>({
            {YGJustifyToString(YGJustifyFlexStart), YGJustifyFlexStart},
            {YGJustifyToString(YGJustifyCenter), YGJustifyCenter},
            {YGJustifyToString(YGJustifyFlexEnd), YGJustifyFlexEnd},
            {YGJustifyToString(YGJustifySpaceBetween), YGJustifySpaceBetween},
            {YGJustifyToString(YGJustifySpaceAround), YGJustifySpaceAround},
        });

        auto ValidAlignValues = makeValidationMap<YGAlign>({
            {YGAlignToString(YGAlignAuto), YGAlignAuto},
            {YGAlignToString(YGAlignFlexStart), YGAlignFlexStart},
            {YGAlignToString(YGAlignCenter), YGAlignCenter},
            {YGAlignToString(YGAlignFlexEnd), YGAlignFlexEnd},
            {YGAlignToString(YGAlignStretch), YGAlignStretch},
            {YGAlignToString(YGAlignBaseline), YGAlignBaseline},
            {YGAlignToString(YGAlignSpaceBetween), YGAlignSpaceBetween},
            {YGAlignToString(YGAlignSpaceAround), YGAlignSpaceAround},
        });

        auto ValidPositionTypeValues = makeValidationMap<YGPositionType>({
            {YGPositionTypeToString(YGPositionTypeRelative), YGPositionTypeRelative},
            {YGPositionTypeToString(YGPositionTypeAbsolute), YGPositionTypeAbsolute},
        });

        auto ValidFlexWrapValues = makeValidationMap<YGWrap>({
            {YGWrapToString(YGWrapNoWrap), YGWrapNoWrap},
            {YGWrapToString(YGWrapWrap), YGWrapWrap},
            {YGWrapToString(YGWrapWrapReverse), YGWrapWrapReverse},
        });

        auto ValidOverflowValues = makeValidationMap<YGOverflow>({
            {YGOverflowToString(YGOverflowVisible), YGOverflowVisible},
            {YGOverflowToString(YGOverflowHidden), YGOverflowHidden},
            {YGOverflowToString(YGOverflowScroll), YGOverflowScroll},
        });

        auto ValidEdgeValues = makeValidationMap<YGEdge>({
            {YGEdgeToString(YGEdgeLeft), YGEdgeLeft},
            {YGEdgeToString(YGEdgeTop), YGEdgeTop},
            {YGEdgeToString(YGEdgeRight), YGEdgeRight},
            {YGEdgeToString(YGEdgeBottom), YGEdgeBottom},
            {YGEdgeToString(YGEdgeStart), YGEdgeStart},
            {YGEdgeToString(YGEdgeEnd), YGEdgeEnd},
            {YGEdgeToString(YGEdgeHorizontal), YGEdgeHorizontal},
            {YGEdgeToString(YGEdgeVertical), YGEdgeVertical},
            {YGEdgeToString(YGEdgeAll), YGEdgeAll},
        });

        //==============================================================================
        template<typename T>
        bool validateFlexProperty (validationMapKey value, std::map<validationMapKey, T> validValues)
        {
            for (const auto& [flexValue, enumValue] : validValues)
            {
                if (value.equalsIgnoreCase(flexValue))
                {
                    return true;
                }
            }

            return false;
        }

    }

    //==============================================================================
    void ShadowView::setProperty (const juce::Identifier& name, const juce::var& newValue)
    {
        props.set(name, newValue);

        //==============================================================================
        // Flex enums
        if (name == juce::Identifier("direction"))
        {
            jassert (validateFlexProperty(newValue, ValidDirectionValues));
            YGNodeStyleSetDirection(yogaNode, ValidDirectionValues[newValue]);
        }

        if (name == juce::Identifier("flexDirection"))
        {
            jassert (validateFlexProperty(newValue, ValidFlexDirectionValues));
            YGNodeStyleSetFlexDirection(yogaNode, ValidFlexDirectionValues[newValue]);
        }

        if (name == juce::Identifier("justifyContent"))
        {
            jassert (validateFlexProperty(newValue, ValidJustifyValues));
            YGNodeStyleSetJustifyContent(yogaNode, ValidJustifyValues[newValue]);
        }

        if (name == juce::Identifier("alignItems"))
        {
            jassert (validateFlexProperty(newValue, ValidAlignValues));
            YGNodeStyleSetAlignItems(yogaNode, ValidAlignValues[newValue]);
        }

        if (name == juce::Identifier("alignContent"))
        {
            jassert (validateFlexProperty(newValue, ValidAlignValues));
            YGNodeStyleSetAlignContent(yogaNode, ValidAlignValues[newValue]);
        }

        if (name == juce::Identifier("alignSelf"))
        {
            jassert (validateFlexProperty(newValue, ValidAlignValues));
            YGNodeStyleSetAlignSelf(yogaNode, ValidAlignValues[newValue]);
        }

        if (name == juce::Identifier("position"))
        {
            jassert (validateFlexProperty(newValue, ValidPositionTypeValues));
            YGNodeStyleSetPositionType(yogaNode, ValidPositionTypeValues[newValue]);
        }

        if (name == juce::Identifier("flexWrap"))
        {
            jassert (validateFlexProperty(newValue, ValidFlexWrapValues));
            YGNodeStyleSetFlexWrap(yogaNode, ValidFlexWrapValues[newValue]);
        }

        if (name == juce::Identifier("overflow"))
        {
            jassert (validateFlexProperty(newValue, ValidOverflowValues));
            YGNodeStyleSetOverflow(yogaNode, ValidOverflowValues[newValue]);
        }

        //==============================================================================
        // Flex dimensions
        if (name == juce::Identifier("flex"))
            BP_SET_FLEX_FLOAT_PROPERTY(newValue, YGNodeStyleSetFlex, yogaNode)
        if (name == juce::Identifier("flexGrow"))
            BP_SET_FLEX_FLOAT_PROPERTY(newValue, YGNodeStyleSetFlexGrow, yogaNode)
        if (name == juce::Identifier("flexShrink"))
            BP_SET_FLEX_FLOAT_PROPERTY(newValue, YGNodeStyleSetFlexShrink, yogaNode)
        if (name == juce::Identifier("flexBasis"))
            BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(newValue, YGNodeStyleSetFlexBasis, yogaNode)
        if (name == juce::Identifier("width"))
            BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(newValue, YGNodeStyleSetWidth, yogaNode)
        if (name == juce::Identifier("height"))
            BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(newValue, YGNodeStyleSetHeight, yogaNode)
        if (name == juce::Identifier("minWidth"))
            BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetMinWidth, yogaNode)
        if (name == juce::Identifier("minHeight"))
            BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetMinHeight, yogaNode)
        if (name == juce::Identifier("maxWidth"))
            BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetMaxWidth, yogaNode)
        if (name == juce::Identifier("maxHeight"))
            BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetMaxHeight, yogaNode)
        if (name == juce::Identifier("aspectRatio"))
            BP_SET_FLEX_FLOAT_PROPERTY(newValue, YGNodeStyleSetAspectRatio, yogaNode)

        //==============================================================================
        // Margin
        juce::Identifier marginMetaProp ("margin");

        for (const auto& [edgeName, enumValue] : ValidEdgeValues)
        {
            juce::Identifier propId (toCamel(validationMapKey("margin-") + edgeName));

            if (name == propId || (name == marginMetaProp && enumValue == YGEdgeAll))
            {
                BP_SET_FLEX_DIMENSION_PROPERTY_AUTO(newValue, YGNodeStyleSetMargin, yogaNode, enumValue);
            }
        }

        //==============================================================================
        // Padding
        juce::Identifier paddingMetaProp ("padding");

        for (const auto& [edgeName, enumValue] : ValidEdgeValues)
        {
            juce::Identifier propId (toCamel(validationMapKey("padding-") + edgeName));

            if (name == propId || (name == paddingMetaProp && enumValue == YGEdgeAll))
            {
                BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetPadding, yogaNode, enumValue);
            }
        }

        //==============================================================================
        // Position
        for (const auto& [edgeName, enumValue] : ValidEdgeValues)
        {
            if (name == juce::Identifier(edgeName))
            {
                BP_SET_FLEX_DIMENSION_PROPERTY(newValue, YGNodeStyleSetPosition, yogaNode, enumValue);
            }
        }
    }

}
