# Preprocessed files

To applying the preprocessing to **all** the files required, in this folder run:

```bash
python3 ./perform_preprocessing.py
```


### Metadata

The `metadata` preprocessed file is the outcome of the rearrangement of lines to
better be parsed later.

**Possible row patterns**

```
BALL,<int: half_of_the_match>,<id>

PLAYER,<str: team (A|B)>,<str: player_name>,<int: left_leg_id>,<int: right_leg_id>,<int: left_arm_id>,<int: right_arm_id>,

REFEREE,None,None,<int: left_leg_id>,<int: right_leg_id>,0,0
```


### Full-game dataset

The `full-game` preprocessed file is the outcome of:

-  addition of EVENT_TYPE column
-  insertion of the game interruptions events in the chronological order


**Events types**

- `**SE**` Sensor event
- `**GI**` Game interruption event


**Creation**

If you want to generate ONLY this single dataset, in this folder run:

```bash
python3 ./preprocess_full_game.py
```

otherwise look at the complete preprocessing run at the top of this file.

**Possible row patterns**

```
SE,<pattern_of_full_game_file>
GI,<int: event_id>,<str: event_name>,<str: event_time_real>,<int: event_timestamp>,<int: event_counter>,<str: comment>
```
