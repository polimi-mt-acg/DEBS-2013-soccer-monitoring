# -*- coding: utf-8 -*-
"""
Apply the preprocessing transformations to all files required.
"""

from preprocess_full_game import process_full_game
from preprocess_metadata import process_metadata

if __name__ == '__main__':
    process_full_game(source_filepath='./../original/full-game')
    process_metadata(source_filepath='./../original/metadata.txt')
