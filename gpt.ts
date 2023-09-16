import OpenAI from "openai";
import { z } from "zod";
import { zodToJsonSchema } from "zod-to-json-schema";

export const generateFunction = async (description: string) => {
  const openai = new OpenAI({
    apiKey: "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", // defaults to process.env["OPENAI_API_KEY"]
  });

  //   const jsonSchema = zodToJsonSchema(schema);

  const response = await openai.chat.completions.create({
    model: "gpt-4",
    messages: [
      {
        role: "system",
        content: `Your job is to generate Javascript code to be executed. Don't say anything, just return the code. DO NOT RETURN IT WRAPPED IN MARKDOWN. DO NOT WRAP THE CODE in \`\`\`javascript\`\`\`
          
          So for a function like this:
          \`\`\`javascript
          function add(a, b) {
            return a + b;
          }
          \`\`\`
          Return the code in an array so it can be turned into a function like this: new Function("a", "b", "return a + b;"):
          \`\`\`
          ["a", "b", "return a + b;"]
          \`\`\`
          
          The last arg is the function body, the rest are the names of the arguments.
          `,
      },
      {
        role: "user",
        // content: `${description}\n\n Use this json schema: ${jsonSchema}`,
        content: `${description}\n\n`,
      },
    ],
  });

  const code = response.choices[0].message.content;
  console.log("Code", code);
  return JSON.parse(code || "");
};
