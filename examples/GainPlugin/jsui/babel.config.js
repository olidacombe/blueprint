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
        allCss: {
          source: "camelCase"
        },
        "kebab-case": [
          "transformRotate" // TODO nice way to beat these
        ]
      }
    ]
  ]
};

