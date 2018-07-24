import sys
from path import Path

game_ts = {
    'half_1': {
        'start': 10753295594424116,
        'end': 12557295594424116,
    },
    'half_2': {
        'start': 13086639146403495,
        'end': 14879639146403495,
    }
}


def generate_test_dataset_top(path_str, out_str, before_start=10, game_rows=50):
    path = Path(path_str)

    # Check file existance
    if not path.isfile():
        raise FileNotFoundError()

    read_lines = []
    out_lines = []
    skipping = True
    counter = 0
    with path.open() as f:
        for line in f:
            line = line.replace('\n', '')

            # While start game event is not found, store each line
            if skipping:
                read_lines.append(line)
                tokens = line.split(',')
                event_t = tokens[0]
                # Upon game start, append to output lines the last BEFORE_START read lines
                if event_t == 'SE' and int(tokens[2]) >= game_ts['half_1']['start']:
                    print('Found starting game line...')
                    print(line)
                    out_lines += read_lines[-before_start:]
                    skipping = False

            # Append GAME_ROWS game event lines
            else:
                if counter < game_rows:
                    out_lines.append(line)
                    counter += 1
                else:
                    break

    # Write on output file
    out_path = Path(out_str)
    with out_path.open(mode='w') as out:
        print('\nWriting test dataset...')
        for line in out_lines:
            print(line)
            out.write(line + '\n')


if __name__ == '__main__':
    args = sys.argv
    usage = 'Usage: {} preprocessed_game_file output_file'.format(args[0])
    try:
        generate_test_dataset_top(args[1], args[2])
    except FileNotFoundError:
        print('Unable to find file {}'.format(args[1]))
        print(usage)
    except IndexError:
        print('Wrong number of parameters...')
        print(usage)
