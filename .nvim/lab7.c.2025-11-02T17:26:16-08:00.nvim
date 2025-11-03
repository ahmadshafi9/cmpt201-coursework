output[i].line_numbers[output[i].count] = input->line_number;
output[i].count++;
found = 1;
break;
}
}

if (found == 0) {
  output[*result_count].doubled_value = input->doubled_value;
  output[*result_count].count = 0;
  output[*result_count].line_numbers[output[*result_count].count] =
      input->line_number;
  output[*result_count].count++;
  (*result_count)++;
}
}

void reduce(Output output) {
  printf("(%d, [", output.doubled_value);

  int i;
  for (i = 0; i < output.count; i++) {
    printf("%d", output.line_numbers[i]);

    if (i < output.count - 1) {
      printf(", ");
    }
  }

  printf("])\n");
}

int main(void) {
  I #include<stdio.h>
#include <string.h>

#define MAX_INPUT_VALUES 100
#define MAX_LINE_NUMBERS 100

      typedef struct {
    int line_number;
    int value;
  } Input;

  typedef struct {
    int line_number;
    int doubled_value;
  } IntermediateInput;

  typedef struct {
    int doubled_value;
    int line_numbers[MAX_LINE_NUMBERS];
    int count;
  } Output;

  void map(Input * input, IntermediateInput * intermediate_input) {
    intermediate_input->doubled_value = input->value * 2;
    intermediate_input->line_number = input->line_number;
  }

  void groupByKey(IntermediateInput * input, Output * output,
                  int *result_count) {
    int i;
    int found = 0;

    for (i = 0; i < *result_count; i++) {
      if (output[i].doubled_value == input->doubled_value) {
        nput inputs[MAX_INPUT_VALUES];
        IntermediateInput intermediate_inputs[MAX_INPUT_VALUES];
        Output grouped_outputs[MAX_INPUT_VALUES];

        int input_count = 0;
        int intermediate_count = 0;
        int output_count = 0;

        char line[10];

        printf("Enter values (one per line). Type 'end' to finish:\n");

        while (fgets(line, sizeof(line), stdin) != NULL) {
          if (strcmp(line, "end\n") == 0) {
            break;
          }

          if (input_count < MAX_INPUT_VALUES) {
            if (sscanf(line, "%d", &inputs[input_count].value) == 1) {
              inputs[input_count].line_number = input_count + 1;
              input_count++;
            }
          } else {
            fprintf(stderr, "Warning: Maximum input values reached.\n");
            break;
          }
        }

        int i;

        // --- Map Phase ---
        for (i = 0; i < input_count; i++) {
          map(&inputs[i], &intermediate_inputs[intermediate_count]);
          intermediate_count++;
        }

        // --- Group Phase ---
        for (i = 0; i < intermediate_count; i++) {
          groupByKey(&intermediate_inputs[i], grouped_outputs, &output_count);
        }

        // --- Reduce Phase ---
        for (i = 0; i < output_count; i++) {
          reduce(grouped_outputs[i]);
        }

        return 0;
      }
