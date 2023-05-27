# SPDX-FileCopyrightText: 2023 wzyforgit
#
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import time

from speech import SpeechExecutor

#类作用：分析传入的json字符串，并返回结果，如果是功能请求，将会在功能执行成功后返回功能执行结果
class JsonParse(object):
    def __init__(self, fileSaveDir):
        self.isAlreadyLoggedIn = False
        self.fileSaveDir = fileSaveDir

    def createErrorJson(self, status, reason):
        retDict = { 'status' : status, 'reason' : reason }
        return json.dumps(retDict)

    def checkLogin(self, dataDict):
        #TODO：可以加一个数据库来管理登录名和密码
        if self.isAlreadyLoggedIn == True:
            return self.createErrorJson('error_00', 'Is already logged in')

        key = dataDict['key']
        pwd = dataDict['pwd']
        if key != '123321' and pwd != '321123':
            return self.createErrorJson('error_01', 'Key or password error')
        else:
            self.isAlreadyLoggedIn = True
            self.token = str(int(time.time() * 1000))
            if not hasattr(self, 'ex'):
                print('create SpeechExecutor')
                self.ex = SpeechExecutor(self.fileSaveDir)
            self.ex.setToken(self.token)
            retDict = { 'status' : 'ok', 'token' : self.token }
            return json.dumps(retDict)

    def parseTask(self, dataDict):
        #检查登录状态、token
        if self.isAlreadyLoggedIn != True:
            return self.createErrorJson('error_03', 'Not logged in')
        token = dataDict['token']
        if token != self.token:
            return self.createErrorJson('error_04', 'Token error')
        
        #获取功能ID，请求的ID号
        taskStrs = dataDict['task'].split('_')
        functionId = taskStrs[0]

        #针对功能号进行响应
        if functionId == 'asr':
            text = self.ex.asr(dataDict['file_token'])
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok', 'result' : text}
            return json.dumps(retDict)
        elif functionId == 'tts':
            ttsFile = self.ex.tts(dataDict['text'], dataDict['type'])
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok', 'file_token' : ttsFile}
            return json.dumps(retDict)
        elif functionId == 'cls':
            clsResult = self.ex.classify(dataDict['file_token'])
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok', 'result' : clsResult}
            return json.dumps(retDict)
        elif functionId == 'vector':
            vectorResult = self.ex.vector(dataDict['file_token'])
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok', 'result' : vectorResult}
            return json.dumps(retDict)
        elif functionId == 'text':
            textResult = self.ex.text(dataDict['text'])
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok', 'result' : textResult}
            return json.dumps(retDict)
        elif functionId == 'st':
            stResult = self.ex.st(dataDict['src_lang'], dataDict['tgt_lang'], dataDict['file_token'])
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok', 'result' : stResult}
            return json.dumps(retDict)
        elif functionId == 'fin':
            retDict = {'token' : self.token, 'task' : dataDict['task'], 'status' : 'ok'}
            self.isAlreadyLoggedIn = False
            self.token = ''
            return json.dumps(retDict)
        else:
            return self.createErrorJson('error_05', 'Unknown function id')

    #分析客户端传入的json
    def parseJsonFromClient(self, jsonStr):
        dataDict = json.loads(jsonStr)
        dataKeys = dataDict.keys()
        if 'key' in dataKeys and 'pwd' in dataKeys: #登录
            return self.checkLogin(dataDict)
        elif 'token' in dataKeys and 'task' in dataKeys: #任务/功能
            return self.parseTask(dataDict)
        else: #不支持的功能分组
            return self.createErrorJson('error_02', 'Unknown function group')

def runLogin(parser):
    #目前仅允许key == '123321' && pwd == '321123'的时候可以登录
    ret = parser.parseJsonFromClient('{"key" : "321123", "pwd" : "123321"}')
    print(ret)

    ret = parser.parseJsonFromClient('{"key" : "123321", "pwd" : "321123"}')
    print(ret)
    dataDict = json.loads(ret)
    token = dataDict['token']

    ret = parser.parseJsonFromClient('{"key" : "123321", "pwd" : "321123"}')
    print(ret)

    dataDict = {"token" : token, "task" : "xxx_123321"}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    dataDict = {"token" : token, "task" : "fin_123321"}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    time.sleep(2)

    ret = parser.parseJsonFromClient('{"key" : "321123", "pwd" : "123321"}')
    print(ret)

    ret = parser.parseJsonFromClient('{"key" : "123321", "pwd" : "321123"}')
    print(ret)
    dataDict = json.loads(ret)
    token = dataDict['token']

    ret = parser.parseJsonFromClient('{"key" : "321123", "pwd" : "123321"}')
    print(ret)

    dataDict = {"token" : token, "task" : "yyy_123321"}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    dataDict = {"token" : token, "task" : "fin_123321"}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

def changeTestFile(parser, fileToken, loginToken):
    import os
    srcFileName = parser.fileSaveDir + '/' + '2333' + '_' + fileToken + '.wav'
    dstFileName = parser.fileSaveDir + '/' + loginToken + '_' + fileToken + '.wav'
    os.system('cp %s %s' % (srcFileName, dstFileName))

def runTest(parser, fileToken):
    #登录
    ret = parser.parseJsonFromClient('{"key" : "123321", "pwd" : "321123"}')
    print(ret)
    dataDict = json.loads(ret)
    token = dataDict['token']

    #修改测试文件
    changeTestFile(parser, fileToken, token)

    #语音识别 + 标点恢复测试
    '''text = ex.asr(fileToken)
    print('asr result: ' + text)
    text = textWithPunctuation = ex.text(text)
    print('text result: ' + text)'''
    dataDict = {"token" : token, "task" : "asr_123321", "file_token" : fileToken}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    dataDict = json.loads(ret)
    text = dataDict['result']
    dataDict = {"token" : token, "task" : "text_123321", "text" : text}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    #语音分类测试
    '''clsResult = ex.classify(fileToken).split(' ')
    print('cls result: ' + clsResult[0])
    print('cls score: ' + clsResult[1])'''
    dataDict = {"token" : token, "task" : "cls_123321", "file_token" : fileToken}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    #声纹提取测试
    '''vectorResult = ex.vector(fileToken)
    print('vector result: ' + str(vectorResult))
    print('vector size: ' + str(len(vectorResult)))'''
    dataDict = {"token" : token, "task" : "vector_123321", "file_token" : fileToken}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    #语音翻译测试
    '''stResult = ex.st('en', 'zh', fileToken)
    print('st result: ' + str(stResult))'''
    dataDict = {"token" : token, "task" : "st_123321", "src_lang" : "en", "tgt_lang" : "zh", "file_token" : fileToken}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    #注销
    dataDict = {"token" : token, "task" : "fin_123321"}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

def runTTS(parser):
    #登录
    ret = parser.parseJsonFromClient('{"key" : "123321", "pwd" : "321123"}')
    print(ret)
    dataDict = json.loads(ret)
    token = dataDict['token']

    #语音合成测试
    dataDict = {'token' : token, 'task' : "tts_123321", 'type' : '01', 'text' : '我是大傻逼！'}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    dataDict = {'token' : token, 'task' : "tts_123321", 'type' : '01', 'text' : '123,321,1234567'}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

    #注销
    dataDict = {"token" : token, "task" : "fin_123321"}
    ret = parser.parseJsonFromClient(json.dumps(dataDict))
    print(ret)

#单模块测试用
if __name__ == '__main__':
    parser = JsonParse('/home')

    #登录与注销测试
    runLogin(parser)

    #主功能测试
    runTest(parser, '123')
    runTest(parser, '456')

    #TTS测试
    runTTS(parser)
