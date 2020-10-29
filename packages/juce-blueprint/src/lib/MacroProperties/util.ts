type TMacroCall = {
  macro: string;
  args: string[]
}

// don't try too hard, we don't need to support qouted strings containing commas etc.
export const splitArgs = (a: string) => a.trim().split(/\s*,\s*/);

export const getMacroCalls = (s: string, candidates?: string[]): TMacroCall[] => {
  const macroMatcher = String.raw`(?<macro>(${candidates ? `${candidates.join('|')}` : '[a-zA-Z0-9]+'}))\((?<args>[^)]*)\)`;
  const r = new RegExp(macroMatcher);
  // ideally named capture groups would get typed, some day
  // @ts-ignore
  return Array.from(s.matchAll(r), ({groups: {macro, args}})=> ({macro, args: splitArgs(args)}))
}

export const degToNumber = (deg: string) => {
  return parseFloat(deg.replace(/deg/i, ''));
}