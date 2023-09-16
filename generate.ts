
import { generateFunction } from "./gpt" with { type: 'macro' }

export async function generateFunction2(description: string) {
    const desc = await generateFunction(description);

    return new Function(
  ...[
    ...desc.slice(0, desc.length - 1),
    desc[Math.max(desc.length - 1, 0)],
  ].filter((v) => !!v))
}