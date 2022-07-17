#!/usr/bin/python3
"""Generate lpmake commands."""
import sys
from pathlib import Path

AB_SUPER_PARTITIONS = {
    'system_a',
    'system_b',
    'product_a',
    'product_b',
    'odm_a',
    'odm_b',
    'vendor_a',
    'vendor_b',
    'system_ext_a',
    'system_ext_b',
}


def main():  # pylint: disable=too-many-branches,too-many-statements
    """Generate lpmake command."""
    print("""
    ******************************
    *  Generate lpmake command   *
    * GH: arichr/android-things  *
    ******************************
    """)
    print('Specify the directory with images.')
    image_dir = Path(input('> '))
    if not image_dir.is_dir():
        print('Wrong directory.')
        sys.exit(1)

    print('[...] Trying to find images...')
    images = tuple(image_dir.glob('*.[iI][mM][gG]'))
    if not images:
        print('No images were found.')
        sys.exit(1)

    print('Which mode do you prefer?')
    mode = input('[ro/rw/ASK]: ')
    if mode in {'readonly', 'Readonly', 'READONLY', 'RO', 'ro'}:
        mode = 'readonly'
    elif mode in {'write', 'Write', 'WRITE', 'RW', 'rw'}:
        mode = 'none'
    else:
        mode = 'ask'

    partitions = []
    images_size = 0
    block_size = 4096
    is_ab = False

    for image_path in images:
        if image_path.stem in AB_SUPER_PARTITIONS:
            is_ab = True

        print(f'*** {image_path.name} ***')
        if mode == 'ask':
            print('Which mode do you prefer?')
            mode = input('[ro/RW]: ')
            if mode in {'readonly', 'Readonly', 'READONLY', 'RO', 'ro'}:
                print('[!] Note: This will be a read-only partition.')
                image_mode = 'readonly'
            else:
                print('[!] Note: This will be a RW partition.')
                image_mode = 'none'

        images_size += image_path.stat().st_size
        partitions.append(
            '--partition {name}:{ro}:{size}:main --image {name}={path}'.format(
                name=image_path.stem,
                ro=image_mode if mode == 'ask' else mode,
                size=image_path.stat().st_size,
                path=str(image_path),
            ),
        )
        print(partitions[-1])

    print('Specify the super partition size in bytes.')
    while True:
        super_size = input('> ')
        if super_size.isdigit() and int(super_size) % block_size == 0:
            super_size = int(super_size)
            break
        print('[!] Wrong super size.')

    print('Should lpmake generate an Android sparse image?')
    is_sparse = input('[y/N]: ')
    if is_sparse in {'Y', 'y'}:
        print('[!] Note: lpmake will output an Android sparse image.')
        is_sparse = '--sparse'
    else:
        print('[!] Note: lpmake will NOT output an Android sprarse image.')
        is_sparse = ''

    if images_size % block_size != 0:
        print('[!] Output super filesize is not correct. Checking images...')
        for image_path in images:
            incomplete_block = image_path.stat().st_size % block_size

            if incomplete_block == 0:
                print(f'[!] Note: {image_path.name} is OK.')
                break

            file_header = tuple(image_path.read_bytes()[:4])
            if file_header == (0x3A, 0xFF, 0x26, 0xED):
                # This is an Android sparse image
                print(
                    f'[!] Note: We are unable to check {image_path.name}',
                    'blocks, due to the usage of sparse images.',
                    "But don't use the raw ones and IGNORE this notice.",
                )
            else:
                print(
                    f'[!] {image_path.name} has a problem',
                    f'(Expected {block_size - incomplete_block} more bytes).',
                )
                sys.exit(1)

    print('[!] Note: metadata-size set to 65536.')
    print('[!] Note: super-name set to super.')

    lpmake_cmd = (
        './lpmake --metadata-size 65536 --super-name super '
        f'--metadata-slots {2 if is_ab else 1} --device super:{super_size} '
        f'--group main:{images_size} {" ".join(partitions)} {is_sparse} '
        f'--output ./super.{hash(tuple(partitions))}.img'
    )

    print('\nResult:\n')
    print(lpmake_cmd)


if __name__ == '__main__':
    main()
