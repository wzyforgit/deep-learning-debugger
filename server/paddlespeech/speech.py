# SPDX-FileCopyrightText: 2023 wzyforgit
#
# SPDX-License-Identifier: GPL-3.0-or-later

from paddlespeech.cli.asr.infer import ASRExecutor   #语音转文字
from paddlespeech.cli.tts.infer import TTSExecutor   #语音合成
from paddlespeech.cli.cls.infer import CLSExecutor   #语音场景分类
from paddlespeech.cli.vector import VectorExecutor   #声纹提取
from paddlespeech.cli.text.infer import TextExecutor #标点恢复
from paddlespeech.cli.st.infer import STExecutor     #语音翻译

import time

class SpeechExecutor(object):
    def __init__(self, dirPath):
        if not dirPath.endswith('/'):
            dirPath = dirPath + '/'
        self.dirPath = dirPath

        #这个玩意儿初始化需要5秒以上，因此得自动预激（在目录下放置一个可用的start.wav音频文件）
        self.asrEx = ASRExecutor()
        self.asrEx(audio_file = 'start.wav', model = 'conformer_talcs', lang='zh_en', codeswitch = True)

    def setToken(self, token):
        self.token = token

    def fileFullName(self, fileNameSuffix):
        return self.dirPath + self.token + '_' + fileNameSuffix + '.wav'

    def createfileNameSuffix(self):
        return str(int(time.time() * 1000))

    def asr(self, fileNameSuffix):
        if not hasattr(self, 'asrEx'):
            print('create ASRExecutor')
            self.asrEx = ASRExecutor()
        filePath = self.fileFullName(fileNameSuffix)
        print('asr decode file: ' + filePath)
        result = self.asrEx(audio_file = filePath, model = 'conformer_talcs', lang='zh_en', codeswitch = True)
        return result

    def tts(self, text, type = '01'):
        #type代表音色，暂时没有启用
        if not hasattr(self, 'ttsEx'):
            print('create TTSExecutor')
            self.ttsEx = TTSExecutor()
        filePath = self.fileFullName(self.createfileNameSuffix())
        print('tts encode file: ' + filePath)
        self.ttsEx(text = text, output = filePath)
        return filePath

    def classify(self, fileNameSuffix):
        if not hasattr(self, 'clsEx'):
            print('create CLSExecutor')
            self.clsEx = CLSExecutor()
        filePath = self.fileFullName(fileNameSuffix)
        print('cls decode file: ' + filePath)
        result = self.clsEx(audio_file = filePath)
        return result

    def vector(self, fileNameSuffix):
        #结果比对的时候使用余弦相似度算法（和arcface一样）
        if not hasattr(self, 'vectorEx'):
            print('create VectorExecutor')
            self.vectorEx = VectorExecutor()
        filePath = self.fileFullName(fileNameSuffix)
        print('vector decode file: ' + filePath)
        result = self.vectorEx(audio_file = filePath)
        return result.tolist()

    def text(self, text):
        if not hasattr(self, 'textEx'):
            print('create TextExecutor')
            self.textEx = TextExecutor()
        result = self.textEx(text = text)
        return result

    def st(self, src, tgt, fileNameSuffix):
        if src != 'en' or tgt != 'zh': #目前只支持英译中
            print(src + ' to ' + tgt + ' is not support now')
            return ''
        if not hasattr(self, 'stEx'):
            print('create STExecutor')
            self.stEx = STExecutor()
        filePath = self.fileFullName(fileNameSuffix)
        print('st decode file: ' + filePath)
        result = self.stEx(audio_file = filePath, src_lang = src, tgt_lang = tgt)
        stResultStr = ''
        for stRet in result:
            stResultStr = stRet + stResultStr + '\n'
        return stResultStr

#单模块测试用
def runTest(ex, fileToken):
    start = time.time()

    #语音识别 + 标点恢复测试
    text = ex.asr(fileToken)
    print('asr result: ' + text)
    text = textWithPunctuation = ex.text(text)
    print('text result: ' + text)

    #语音分类测试
    clsResult = ex.classify(fileToken).split(' ')
    print('cls result: ' + clsResult[0])
    print('cls score: ' + clsResult[1])

    #声纹提取测试
    vectorResult = ex.vector(fileToken)
    print('vector result: ' + str(vectorResult))
    print('vector size: ' + str(len(vectorResult)))

    #语音翻译测试
    stResult = ex.st('en', 'zh', fileToken)
    print('st result: ' + stResult)

    print('time usage: ' + str(time.time() - start))

def runTTS(ex):
    start = time.time()
    ex.tts('123,321,1234567')
    print('time usage: ' + str(time.time() - start))

    start = time.time()
    ex.tts('我是大傻逼！')
    print('time usage: ' + str(time.time() - start))

if __name__ == '__main__':
    ex = SpeechExecutor('/home')
    ex.setToken('2333')

    runTest(ex, '123')
    runTest(ex, '456')
    runTTS(ex)
