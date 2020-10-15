module.exports = {
  presets: [
    [
      "@babel/preset-env",
      {
        "modules": "umd"
      }
    ],
    "@babel/preset-react",
  ],
  plugins: [
    "@babel/plugin-proposal-class-properties",
    [
      "@babel/plugin-transform-runtime",
      {
        "absoluteRuntime": false,
        "corejs": 3,
        "version": "7.11.2"
      }
    ],
    [
      "babel-plugin-transform-property-cases",
      {
        "kebab-case": [
          "alignItems",
          "aspectRatio",
          "backgroundColor",
          "flexDirection",
          "fontSize",
          "lineSpacing",
          "justifyContent",
          "maxWidth",
          "minHeight",
          "minWidth",
          "transformRotate"
        ]
      }
    ]
  ]
};

