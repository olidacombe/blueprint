import { getMacroCalls, degToNumber } from "./util";

type TPropertyAssignment = [property: string, value: string | number];

export default function (value: string): TPropertyAssignment[] {
  const calls = getMacroCalls(value, ["rotate"]);
  const propertyAssignments: TPropertyAssignment[] = [];
  for (const { macro, args } of calls) {
    if (macro === "rotate" && args.length > 0) {
      const degreesNumber = degToNumber(args[0]);
      if (degreesNumber !== NaN)
        propertyAssignments.push(["transform-rotate", degreesNumber]);
    }
  }
  return propertyAssignments;
}
