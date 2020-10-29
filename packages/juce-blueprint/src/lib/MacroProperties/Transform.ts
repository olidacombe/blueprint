import { getMacroCalls, degToNumber } from './util';

type TPropertySetter = [macro: string, arg: string|number];

export default function(value: string): TPropertySetter[] {
  const calls = getMacroCalls(value, ["rotate"]);
  const propertySetters: TPropertySetter[] = [];
  for(const {macro, args} of calls) {
    if(macro === "rotate" && args.length > 0) {
      const degreesNumber = degToNumber(args[0]);
      if(degreesNumber !== NaN)
        propertySetters.push(['transform-rotate', degreesNumber]);
    }
  }
  return propertySetters;
}