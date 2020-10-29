type TMacroCall = {
  macro: string;
  args: string[];
};

// don't try too hard, we don't need to support qouted strings containing commas etc.
export const splitArgs = (a: string) => a.trim().split(/\s*,\s*/);

export const getMacroCalls = (
  s: string,
  candidates?: string[]
): TMacroCall[] => {
  const macroMatcher = String.raw`(${
    candidates ? `${candidates.join("|")}` : "[a-zA-Z0-9]+"
  })\(([^)]*)\)`;
  const r = new RegExp(macroMatcher);
  // ideally named capture groups would get typed, some day
  // @ts-ignore
  // return Array.from(s.matchAll(r), ([, macro, args]) => ({
  //   macro,
  //   args: splitArgs(args),
  // }));

  // until I sort out the lack of matchAll
  // in duktape, this proves the rest works a bit
  const [, macro, args] = r.exec(s);
  return [
    {
      macro,
      args: splitArgs(args),
    },
  ];
};
