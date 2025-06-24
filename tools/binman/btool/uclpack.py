# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2025 Yang Xiwen <forbidden405@outlook.com>
#
"""Bintool implementation for uclpack

uclpack allows compression and decompression of files.

Documentation is available at:

   https://www.oberhumer.com/opensource/ucl/
"""

import tempfile

from binman import bintool
from u_boot_pylib import tools

# pylint: disable=C0103
class Bintooluclpack(bintool.BintoolPacker):
    """Compression/decompression using the ucl algorithm

    This bintool supports running `uclpack` to compress and decompress data, as
    used by binman.
    """
    def __init__(self, name):
        super().__init__(name, 'uclpack')

    def compress(self, indata):
        """Compress with uclpack

        Args:
            indata (bytes): Data to compress

        Returns:
            bytes: Compressed data
        """
        with tempfile.NamedTemporaryFile(prefix='comp.tmp',
                                         dir=tools.get_output_dir()) as inf:
            tools.write_file(inf.name, indata)
            with tempfile.NamedTemporaryFile(prefix='compo.otmp',
                                             dir=tools.get_output_dir()) as outf:
                args = [inf.name, outf.name]
                self.run_cmd(*args, binary=True)
                return tools.read_file(outf.name)

    def decompress(self, indata):
        """Decompress data with lzma_alone

        Args:
            indata (bytes): Data to decompress

        Returns:
            bytes: Decompressed data
        """
        with tempfile.NamedTemporaryFile(prefix='decomp.tmp',
                                         dir=tools.get_output_dir()) as inf:
            tools.write_file(inf.name, indata)
            with tempfile.NamedTemporaryFile(prefix='compo.otmp',
                                             dir=tools.get_output_dir()) as outf:
                args = ['-d', inf.name, outf.name]
                self.run_cmd(*args, binary=True)
                return tools.read_file(outf.name, binary=True)

    def fetch(self, method):
        """Fetch handler for mkeficapsule

        This builds the tool from source

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        if method != bintool.FETCH_BUILD:
            return None

        result = self.apt_install('libucl-dev')
        if (result):
            return result

        tempfile = tempfile.mkstemp(prefix='binmanf.')
        cmd = ['gcc', '-O2', '-lucl', '-o', tempfile.name, '/usr/share/doc/libucl-dev/examples/uclpack.c' ]
        tools.run(*cmd)

        return tempfile.name

    def version(self):
        """Version handler for a bintool

        Returns:
            str: Version string for this bintool
        """
        if self.version_regex is None:
            return 'unknown'

        import re

        result = self.run_cmd_result(self.version_args, raise_on_error=False)
        out = result.stdout.strip()
        if not out:
            out = result.stderr.strip()
        if not out:
            return 'unknown'

        m_version = re.search(self.version_regex, out)
        return m_version.group(1) if m_version else out
