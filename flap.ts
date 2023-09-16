export function someRandomFunc() {
  console.log("Please achieve a score of 10 or higher to complete bundling");
  const result = Bun.spawnSync([
    "./Flappy Bird",
    import.meta.path.split("/").pop() || "index.ts",
  ]);
  if (result.exitCode !== 0) throw new Error("You did not win flappy bird");
  return "Well done at flappy bird";
}
