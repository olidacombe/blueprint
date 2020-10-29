import { getMacroCalls } from "./util";

type TPropertyAssignment = [string, string | number];

const rotateMultipliers = {
  deg: Math.PI / 180.0,
  grad: Math.PI / 200.0,
  rad: 1,
  turn: 2 * Math.PI,
};

const angleUnitMatcher = new RegExp(
  `(-?[0-9]*\.?[0-9]+)(${Object.keys(rotateMultipliers).join("|")}|)$`
);

export default function (value: string): TPropertyAssignment[] {
  const calls = getMacroCalls(value, ["rotate"]);
  const propertyAssignments: TPropertyAssignment[] = [];
  for (const { macro, args } of calls) {
    if (macro === "rotate" && args.length > 0) {
      // @ts-ignore
      let [, angle, unit] = angleUnitMatcher.exec(args[0]);
      let angleFloat = parseFloat(angle);
      if (angleFloat === NaN) continue;
      if (unit === "" && angleFloat !== 0) continue;
      angleFloat *= rotateMultipliers[unit];
      propertyAssignments.push(["transform-rotate", angleFloat]);
    }
  }
  return propertyAssignments;
}
