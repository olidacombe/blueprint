import { TPropertyAssignment } from "./types";
import { getMacroCalls } from "./util";
import { add, identity, matrix, multiply, rotationMatrix } from 'mathjs';

const rotateMultipliers = {
  deg: Math.PI / 180.0,
  grad: Math.PI / 200.0,
  rad: 1,
  turn: 2 * Math.PI,
};

const angleUnitMatcher = new RegExp(
  `(-?[0-9]*\.?[0-9]+)(${Object.keys(rotateMultipliers).join("|")}|)$`
);

// this may be overkill, mathjs ops seem immutable
const getIdentityTransform = () => identity(4);

const toRadians = (arg: string) => {
  // @ts-ignore
  let [, angle, unit] = angleUnitMatcher.exec(arg);
  let angleFloat = parseFloat(angle);
  if (angleFloat === NaN) return;
  if (unit === "" && angleFloat !== 0) return;
  angleFloat *= rotateMultipliers[unit] || 1;
  return angleFloat;
}

const argsToRadians = f => (...args) => f(...args.map(a => toRadians(a)));

const oneInTheCorner = matrix([
  [0, 0, 0, 0],
  [0, 0, 0, 0],
  [0, 0, 0, 0],
  [0, 0, 0, 1]
]);
const toHomogeneous = m => add(m, oneInTheCorner);

const rotate = argsToRadians((theta) => toHomogeneous(rotationMatrix(theta)));
const translate = (dx = 0, dy = 0, dz = 0) => toHomogeneous(
  matrix([
    [1, 0, 0, dx],
    [0, 1, 0, dy],
    [0, 0, 1, dz],
    [0, 0, 0, 1]
  ])
);

const transformFunctionMap = {
  rotate,
  rotateZ: rotate,
  translate
}

export default function (value: string): TPropertyAssignment[] {
  const calls = getMacroCalls(value, Object.keys(transformFunctionMap));
  // TODO check we're accumulating calls in the right order, otherwise reverse
  //@ts-ignore
  const transformMatrix = calls.reduce((acc, [f, args]) => {
    const transform = transformFunctionMap[f](...args);
    return transform ? multiply(transform, acc) : acc;
  }, getIdentityTransform());

  return [['transform-matrix', transformMatrix]];
}
