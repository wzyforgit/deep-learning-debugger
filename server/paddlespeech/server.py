# SPDX-FileCopyrightText: 2023 wzyforgit
#
# SPDX-License-Identifier: GPL-3.0-or-later

from paddlespeech.cli.st.infer import STExecutor
st = STExecutor()
#result = st(audio_file="/home/en.wav")
#print('st result: ' + result)
result = st(audio_file="/home/zh.wav", src_lang="en", tgt_lang="zh")
print(result)
