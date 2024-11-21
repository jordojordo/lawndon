import * as math from 'mathjs';

export function calculateTagPosition(
  positions: [number, number][],
  distances: number[]
): [number, number] {
  if (positions.length < 3) {
    console.error('At least three anchors are required.');
    return [-1, -1];
  }

  try {
    const [x1, y1] = positions[0];
    const [x2, y2] = positions[1];
    const [x3, y3] = positions[2];
    const r1 = distances[0];
    const r2 = distances[1];
    const r3 = distances[2];

    // Formulate matrices and solve for position
    const A = [
      [2 * (x2 - x1), 2 * (y2 - y1)],
      [2 * (x3 - x1), 2 * (y3 - y1)],
    ];
    const B = [
      r1 ** 2 - r2 ** 2 - x1 ** 2 + x2 ** 2 - y1 ** 2 + y2 ** 2,
      r1 ** 2 - r3 ** 2 - x1 ** 2 + x3 ** 2 - y1 ** 2 + y3 ** 2,
    ];

    const position = math.lusolve(A, B) as number[][];

    const x = position[0][0];
    const y = position[1][0];

    return [x, y];
  } catch (error) {
    console.error('Error calculating position:', error);
    return [-1, -1];
  }
}
