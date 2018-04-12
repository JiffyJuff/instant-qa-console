# instant-qa-console
A simple question-and-answer C application for the command-line interface.

FORMAT:

`./a.out [path] [start] [end] [length]`

ARGUMENTS:
- `[path]`: the file path to the target .csv
- `[start]`: which line to start reading from (inclusive).
- `[end]`: which line to stop reading (inclusive).
- `[length]`: optional parameter to control character limits
  - The default character limit for questions and answers is 70 each
  - The default character limit for each line (question + answer) is 100
  - Passing "long" as the `[length]` argument doubles both limits.
  - Passing "superlong" as the `[length]` argument quadruples both limits.

The target csv should be formatted as such:
```
question1,answer1
question2,answer2
question3,answer3
...
```

An example:
```
acceleration,derivative of velocity with respect to time
velocity,derivative of displacement with respect to time
Newton's Second Law,F=ma
...
```

Special and accented characters are accepted. Commas are not accepted in the question segment and will result in an unreported parsing error, but allowed in the answer segment. Answers are case-sensitive.

Submit "q" as an answer to any question to quit immediately.
