# Preprocessed datasets

To applying the preprocessing to **all** the files required, in this folder run:

```bash
python3 ./perform_preprocessing.py
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

**Notes**

On the preprocessed file:

- row #