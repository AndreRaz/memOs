#ifndef OUTPUT_H
#define OUTPUT_H

void output_header(const char *cols[], int n);
void output_row(const char *vals[], int n);
void output_separator(int n);
void output_memory_row(int id, const char *type,
                       double decay, const char *content);

#endif