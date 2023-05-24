'''
启动语音识别会话
发送：
{
    "key" : "123321"
    "pwd" : "321123"
}
接收：
{
    "status" : "ok"
    "token"  : "2333"
}

启动一次语音任务
识别
发送：
{
    "token" : "2333"
    "task"  : "asr_taskid"
    "file_token" : "123" //完整文件名应该是: 2333_123.wav，存放于服务器指定的路径下
}
接收：
{
    "token"  : "2333"
    "task"   : "asr_taskid"
    "status" : "ok"
    "result" : "我认为跑步最重要的就是给我带来了身体健康"
}

合成
发送：
{
    "token" : "2333"
    "task"  : "tts_taskid"
    "text"  : "123321"
}
接收：
{
    "token"  : "2333"
    "task"   : "tts_taskid"
    "status" : "ok"
    "file_token" : "123"
}

分类
发送：
{
    "token" : "2333"
    "task"  : "cls_taskid"
    "file_token" : "123" //完整文件名应该是: 2333_123.wav，存放于服务器指定的路径下
}
接收：
{
    "token"  : "2333"
    "task"   : "cls_taskid"
    "status" : "ok"
    "result" : "Speech 0.9027186632156372"
}

声纹
发送：
{
    "token" : "2333"
    "task"  : "vector_taskid"
    "file_token" : "123" //完整文件名应该是: 2333_123.wav，存放于服务器指定的路径下
}
接收：
{
    "token"  : "2333"
    "task"   : "vector_taskid"
    "status" : "ok"
    "result" : [-0.19083306,9.474295,-14.122263,-2.0916545 ...]
}

标点恢复
发送：
{
    "token" : "2333"
    "task"  : "text_taskid"
    "text" :  "123"
}
接收：
{
    "token"  : "2333"
    "task"   : "text_taskid"
    "status" : "ok"
    "result" : "1,2,3."
}

翻译，目前只支持en2zh
发送：
{
    "token" : "2333"
    "task"  : "st_taskid"
    "src_lang" : "en"
    "tgt_lang" : "zh"
    "file_token" : "123" //完整文件名应该是: 2333_123.wav，存放于服务器指定的路径下
}
接收：
{
    "token"  : "2333"
    "task"   : "st_taskid"
    "status" : "ok"
    "result" : "123321"
}

结束语音识别会话
发送：
{
    "token" : "2333"
    "task"  : "fin_taskid"
}
接收：
{
    "token" : "2333"
    "task"  : "fin_taskid"
    "status": "ok"
}
'''

'''
from paddlespeech.cli.asr.infer import ASRExecutor
asr = ASRExecutor()
result = asr(audio_file="/home/zh.wav", model="conformer_talcs", lang="zh_en", codeswitch=True)
print('rec result: ' + result)
result = asr(audio_file="/home/en.wav", model="conformer_talcs", lang="zh_en", codeswitch=True)
print('rec result: ' + result)
'''

from paddlespeech.cli.st.infer import STExecutor
st = STExecutor()
#result = st(audio_file="/home/en.wav")
#print('st result: ' + result)
result = st(audio_file="/home/zh.wav", src_lang="en", tgt_lang="zh")
print(result)
