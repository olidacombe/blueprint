import { TMacroCall } from "./types";

// don't try too hard, we don't need to support qouted strings containing commas etc.
export const splitArgs = (a: string) => a.trim().split(/\s*,\s*/);

export const getMacroCalls = (
  s: string,
  candidates?: string[]
): TMacroCall[] => {
  const macroMatcher = String.raw`(${
    candidates ? `${candidates.join("|")}` : "[a-zA-Z0-9]+"
  })\(([^)]*)\)`;
  const r = new RegExp(macroMatcher, "g");
  // the rest would be trivial with matchAll
  // which duktape sadly lacks
  const matches = s.match(r);
  if (!matches) return [];
  const macroCalls = [];
  for (const match of matches) {
    // @ts-ignore
    const [, macro, args] = r.exec(match);
    macroCalls.push({
      // @ts-ignore
      macro,
      // @ts-ignore
      args: splitArgs(args),
    });
  }
  return macroCalls;
};
