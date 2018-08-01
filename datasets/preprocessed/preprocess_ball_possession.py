import os
import csv
from pathlib import Path

FIELDNAMES = ['event_id', 'event_name', 'event_time_real', 'event_counter',
              'comment']


def str_to_seconds(t):
    if len(t) == 12:
        hh = int(t[0:2])
        mm = int(t[3:5])
        ss = int(t[6:8])
        ms = int(t[9:12])

        seconds = 0
        seconds += hh * 60 * 60
        seconds += mm * 60
        seconds += ss
        seconds += ms * int(1e-3)
        return seconds


def load_possession_content(source_path):
    path = Path(source_path)
    if not path.is_dir():
        raise NotADirectoryError('{} is not a directory'.format(source_path))

    possession = {}

    csv_files = list(path.glob('**/*.csv'))
    for csv_file in csv_files:
        with open(str(csv_file), newline='', encoding='utf-8') as f:
            reader = csv.DictReader(f, fieldnames=FIELDNAMES, delimiter=';')
            rows = [x for x in reader if
                    x['comment'] is not None and x['comment'] != '']
            rows = rows[1:]

            if len(rows) % 2 != 0:
                print(rows)
            print('Parsing {} file'.format(csv_file.name))
            pairs = [(rows[i], rows[i + 1]) for i in range(0, len(rows), 2)]
            for b, e in pairs:
                v = possession.setdefault(csv_file.stem, 0.0)
                b_time = b['event_time_real']
                e_time = e['event_time_real']
                b_seconds = str_to_seconds(b_time)
                e_seconds = str_to_seconds(e_time)
                v += e_seconds - b_seconds
                possession[csv_file.stem] = v

    total_seconds = 0.0
    for v in possession.values():
        total_seconds += v

    # Scale to 1
    for k, v in possession.items():
        possession[k] = v / total_seconds

    return ['{} {:02.02f}'.format(k, v * 100) for k, v in possession.items()]


def process_possession(source_path='', target_filepath='game_statistics'):
    '''
    Perform preprocessing of metadata file
    '''

    print('=============================================')
    print(f'  {source_path} FILE PROCESSING')
    print('=============================================')
    print('Starting preprocessing...')
    print('#lines\tFile')
    os.system(f'wc -l {source_path}')

    lines = load_possession_content(source_path=source_path)

    # add line separator
    lines = map(lambda x: x + '\n', lines)

    # open target file
    with open(target_filepath, 'w') as out:
        out.writelines(lines)

    print('\n================================================================')
    print(f'  {source_path} FILE PROCESSING COMPLETED')
    print(
        '================================================================\n\n')


if __name__ == '__main__':
    process_possession(
        source_path='./../original/referee-events/Ball Possession/')
