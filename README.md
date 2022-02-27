## Readers-Writers problem solution

### Using POSIX API

> Writers-preference.
> "No writer, once added to the queue, shall be kept waiting longer than absolutely necessary."

> Количество читателей, писателей и повторений, задержка, а также возможность
> случайно генерировать задержку (добавляя случайное число от 0-3
> к установленной) чтения/записи устанавливаются директивами препроцессора.

> Readers and writers amount, read/write delay or random delay generation option (adding 0-3 to user-defined) can be changed by #define directives
