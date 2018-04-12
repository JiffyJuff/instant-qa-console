# instant-qa-console
A simple question-and-answer C application for the command-line interface.

### Format:

`./a.out [path] [start] [end] [length]`

### Arguments:
- `[path]`: the file path to the target .csv
- `[start]`: which line to start reading from (inclusive).
- `[end]`: which line to stop reading (inclusive).
- `[length]`: optional parameter to control character limits
  - The default character limit for questions and answers is 70 each
  - The default character limit for each line (question + answer) is 100
  - Passing "long" as the `[length]` argument doubles both limits.
  - Passing "superlong" as the `[length]` argument quadruples both limits.

### Formatting input

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

Special and accented characters are accepted. Commas are not accepted in the question segment and will result in an unreported parsing error, but allowed in the answer segment. Answers are case- and whitespace-sensitive.

### Instructions of use

The lines selected in the initial arguments are loaded into a 'deck' of question/answer pairs. They are dealt randomly. The user is to enter what they believe to be the correct answer and submit by pressing the return key. The suggested answer is then displayed; a correct response is indicated by green, an incorrect response is indicated by red, and a partially correct response is indicated by yellow.

The progress through the current deck is recorded to the right of the screen as `[?/?]`, incrementing with each response whether correct or not. When the deck is exhausted, correctly answered questions are removed from the deck and the remaining 'cards' are reshuffled and dealt again. This repeats until all questions have been answered correctly.

When the deck is empty (all questions correctly answered), the user is informed of the number of questions answered correctly in the first round, as well as the number of questions completed thus far. The deck is then reloaded to its initial state and the cycle repeats.

Submit `\s` as an  answer to skip that question. It is removed from the deck.

Submit `\q` as an answer to any question to quit immediately.
