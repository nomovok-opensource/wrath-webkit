# Copyright (C) 2010 Chris Jerdonek (cjerdonek@webkit.org)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This module is required for Python to treat this directory as a package.

"""Autoinstalls third-party code required by WebKit."""

from __future__ import with_statement

import codecs
import os
import sys

from webkitpy.common.system.autoinstall import AutoInstaller
from webkitpy.common.system.filesystem import FileSystem

_THIRDPARTY_DIR = os.path.dirname(__file__)
_AUTOINSTALLED_DIR = os.path.join(_THIRDPARTY_DIR, "autoinstalled")

# Putting the autoinstall code into webkitpy/thirdparty/__init__.py
# ensures that no autoinstalling occurs until a caller imports from
# webkitpy.thirdparty.  This is useful if the caller wants to configure
# logging prior to executing autoinstall code.

# FIXME: If any of these servers is offline, webkit-patch breaks (and maybe
# other scripts do, too). See <http://webkit.org/b/42080>.

# We put auto-installed third-party modules in this directory--
#
#     webkitpy/thirdparty/autoinstalled
fs = FileSystem()
fs.maybe_make_directory(_AUTOINSTALLED_DIR)

init_path = fs.join(_AUTOINSTALLED_DIR, "__init__.py")
if not fs.exists(init_path):
    fs.write_text_file(init_path, "")

readme_path = fs.join(_AUTOINSTALLED_DIR, "README")
if not fs.exists(readme_path):
    fs.write_text_file(readme_path,
        "This directory is auto-generated by WebKit and is "
        "safe to delete.\nIt contains needed third-party Python "
        "packages automatically downloaded from the web.")


class AutoinstallImportHook(object):
    def __init__(self, filesystem=None):
        self._fs = filesystem or FileSystem()

    def find_module(self, fullname, path):
        # This method will run before each import. See http://www.python.org/dev/peps/pep-0302/
        if '.autoinstalled' not in fullname:
            return

        if '.mechanize' in fullname:
            self._install_mechanize()
        elif '.pep8' in fullname:
            self._install_pep8()
        elif '.eliza' in fullname:
            self._install_eliza()
        elif '.irc' in fullname:
            self._install_irc()
        elif '.pywebsocket' in fullname:
            self._install_pywebsocket()

    def _install_mechanize(self):
        # The mechanize package uses ClientForm, for example, in _html.py.
        # Since mechanize imports ClientForm in the following way,
        #
        # > import sgmllib, ClientForm
        #
        # the search path needs to include ClientForm.  We put ClientForm in
        # its own directory so that we can include it in the search path
        # without including other modules as a side effect.
        clientform_dir = self._fs.join(_AUTOINSTALLED_DIR, "clientform")
        installer = AutoInstaller(append_to_search_path=True,
                                  target_dir=clientform_dir)
        installer.install(url="http://pypi.python.org/packages/source/C/ClientForm/ClientForm-0.2.10.zip",
                          url_subpath="ClientForm.py")

        self._install("http://pypi.python.org/packages/source/m/mechanize/mechanize-0.2.4.zip",
                      "mechanize")

    def _install_pep8(self):
        self._install("http://pypi.python.org/packages/source/p/pep8/pep8-0.5.0.tar.gz#md5=512a818af9979290cd619cce8e9c2e2b",
                      "pep8-0.5.0/pep8.py")

    def _install_eliza(self):
        installer = AutoInstaller(target_dir=_AUTOINSTALLED_DIR)
        installer.install(url="http://www.adambarth.com/webkit/eliza",
                          target_name="eliza.py")

    def _install_irc(self):
        # Since irclib and ircbot are two top-level packages, we need to import
        # them separately.  We group them into an irc package for better
        # organization purposes.
        irc_dir = self._fs.join(_AUTOINSTALLED_DIR, "irc")
        installer = AutoInstaller(target_dir=irc_dir)
        installer.install(url="http://downloads.sourceforge.net/project/python-irclib/python-irclib/0.4.8/python-irclib-0.4.8.zip",
                          url_subpath="irclib.py")
        installer.install(url="http://downloads.sourceforge.net/project/python-irclib/python-irclib/0.4.8/python-irclib-0.4.8.zip",
                          url_subpath="ircbot.py")

    def _install_pywebsocket(self):
        pywebsocket_dir = self._fs.join(_AUTOINSTALLED_DIR, "pywebsocket")
        installer = AutoInstaller(target_dir=pywebsocket_dir)
        installer.install(url="http://pywebsocket.googlecode.com/files/mod_pywebsocket-0.5.2.tar.gz",
                          url_subpath="pywebsocket-0.5.2/src/mod_pywebsocket")

    def _install(self, url, url_subpath):
        installer = AutoInstaller(target_dir=_AUTOINSTALLED_DIR)
        installer.install(url=url, url_subpath=url_subpath)


sys.meta_path.append(AutoinstallImportHook())