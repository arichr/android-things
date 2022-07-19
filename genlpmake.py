#!/usr/bin/python3
"""Generate lpmake commands."""
import sys
from pathlib import Path

SPARSE_IMAGE_HEADER = (0x3A, 0xFF, 0x26, 0xED)
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


def ask_sparse_image(raw_image_path: Path) -> (Path, Path):
    """Ask for a sparse image.

    Args:
        raw_image_path: Raw image Path

    Returns:
        (Path, Path): The raw image's Path and the sparse image's Path
    """
    print(
        f'[!] {raw_image_path.name} is an raw image.',
        'To proceed, create an Android sparse image:',
        f'img2simg {raw_image_path} {raw_image_path}.sparse.img\n',
    )
    while True:
        sparse_image_path = Path(input('> '))
        if sparse_image_path.is_file():
            file_header = tuple(sparse_image_path.read_bytes()[:4])
            if file_header == SPARSE_IMAGE_HEADER:
                break

            print(
                f'[!] {sparse_image_path.name} is',
                'NOT an Android sparse image.',
            )
        else:
            print('[!] Wrong file location.')

    if raw_image_path == sparse_image_path:
        print(
            '[!] It seems, you overwritten the raw image.',
            "Please, DON'T do it again.",
        )
        return ask_raw_image(sparse_image_path), sparse_image_path

    return raw_image_path, sparse_image_path


def ask_raw_image(sparse_image_path: Path) -> (Path, Path):
    """Ask for a raw image.

    Args:
        sparse_image_path: Android sparse image Path

    Returns:
        (Path, Path): The raw image's Path and the sparse image's Path
    """
    print(
        f'[!] {sparse_image_path.name} is an Android sparse image.',
        'To proceed, extract a raw image:',
        f'simg2img {sparse_image_path} {sparse_image_path}.raw.img\n',
    )
    while True:
        raw_image_path = Path(input('> '))
        if raw_image_path.is_file():
            if tuple(raw_image_path.read_bytes()[:4]) == SPARSE_IMAGE_HEADER:
                print(f'[!] {raw_image_path.name} is an Android sparse image.')
            else:
                break
        else:
            print('[!] Wrong file location.')

    if raw_image_path == sparse_image_path:
        print(
            '[!] It seems, you overwritten the sparse image.',
            "Please, DON'T do it again.",
        )
        return raw_image_path, ask_sparse_image(raw_image_path)

    return raw_image_path, sparse_image_path


def get_partition_images(image_path: Path) -> (Path, Path):
    """For lpmake we need to work with both sparse and raw image at once.

    Args:
        image_path: Image Path (can be a sparse or raw one)

    Returns:
        (Path, Path): The raw image's Path and the sparse image's Path
    """
    file_header = tuple(image_path.read_bytes()[:4])
    if file_header == SPARSE_IMAGE_HEADER:
        return ask_raw_image(image_path)

    return ask_sparse_image(image_path)


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
    images = list(image_dir.glob('*.[iI][mM][gG]'))
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
    raw_images_paths = []
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

        print(
            '[!] Note: Keep the sparse image name clean',
            "(e.g. 'system_a.img').",
        )
        # Note: Unlike raw_images_paths, images can contain
        # either sparse or raw images.
        raw_image_path, image_path = get_partition_images(image_path)
        raw_images_paths.append(raw_image_path)

        images_size += raw_image_path.stat().st_size
        partitions.append(
            '--partition {name}:{ro}:{size}:main --image {name}={path}'.format(
                name=image_path.stem,
                ro=image_mode if mode == 'ask' else mode,
                size=raw_image_path.stat().st_size,
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
        print('[!] Note: lpmake will NOT output an Android sparse image.')
        is_sparse = ''

    if images_size > super_size:
        print('[!] Your images are too big to fit in the super partition.')
        sys.exit(1)
    elif super_size - images_size < 104857600:  # 104857600 bytes = 100 mb
        print(
            '[!] Note: Free super space is less than 100mb.',
            'This may cause device-specific issues while booting Android.',
        )

    if images_size % block_size != 0:
        print('[!] Output super filesize is not correct. Checking images...')
        for image_path in raw_images_paths:
            incomplete_block = image_path.stat().st_size % block_size

            if incomplete_block == 0:
                print(f'[!] Note: {image_path.name} is OK.')
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
