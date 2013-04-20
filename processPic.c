/*
 * 获取图片hash指纹
 * 王星友
 * 2013年寒假
 */
#include <stdio.h>
#include "mpi.h"
#include "getFileName.h"
#include "imagHelper.h"

#define FILEPATH  "/home/nlp/projects/pic/imagProcess/"
int main(argc,argv)
    int argc;
    char **argv;
{
  int node,nodeSize;
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&node);
  MPI_Comm_size(MPI_COMM_WORLD,&nodeSize);
  if(node == 0){
    master();
  }else{
    slaver();
  }

  MPI_Finalize();
  return 0;
}
#define COM_TAG 0
#define MES_TAG 1
#define MES_FINISH 2
int master(){
  int size,rank,nslave,buf;
  MPI_Status status;
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  nslave = size-1;
  _fileNameNode *pFile,*pCur;
  pFile = pCur = getFileNameByDir(FILEPATH);
  while(nslave>0){
    //printf("主线程等待请求\n");
    MPI_Recv(&rank,1,MPI_INT,MPI_ANY_SOURCE,COM_TAG,MPI_COMM_WORLD,&status);
    //printf("主线程收到%d的请求\n",rank);
    if(pCur){
      //还有未处理的信息
      //send message
      //printf("有任务，主线程发送回应给%d\n",rank);
      MPI_Send(&rank,1,MPI_INT,rank,COM_TAG,MPI_COMM_WORLD);
      //printf("有任务，主线程发送回应给%d完毕\n",rank);
      //printf("有任务，主线程发送任务给%d\n",rank);
      MPI_Send(pCur->fileName,strlen(pCur->fileName)+1,MPI_CHAR,rank,
            MES_TAG,MPI_COMM_WORLD);
      //printf("有任务，主线程发送任务给%d完毕\n",rank);
      //printf("pCur:%d\n",(long)pCur);
      pCur = pCur->next;
    }else{
      //处理完了,告诉slave处理完了
      //printf("没有任务，主线程发送回应给%d\n",rank);
      MPI_Send(&rank,1,MPI_INT,rank,MES_FINISH,MPI_COMM_WORLD);
      nslave--;
    }
  }
  freefileNameList(pFile);
  pFile = pCur = NULL;
  return 0;
}
int slaver(){
  char buf[256];
  MPI_Status status;
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  while(1){
    /*轮询获取消息*/
    //printf("%d 线程发送请求任务\n",rank);
    MPI_Send(&rank,1,MPI_INT,0,COM_TAG,MPI_COMM_WORLD);
    //printf("%d 线程发送请求任务完毕\n",rank);
    //printf("%d 线程等待接收回应\n",rank);
    MPI_Recv(&rank,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    //printf("%d 线程接收回应完毕,回应内容为:%d\n",rank,status.MPI_TAG);
    if( status.MPI_TAG == MES_FINISH ){
      //printf("I am done %d\n",rank);
      break;
    }else{
      //printf("%d 线程等待接收任务\n",rank);
      MPI_Recv(buf,256,MPI_CHAR,0,MES_TAG,MPI_COMM_WORLD,&status);
      //printf("%d 线程接收任务完毕\n",rank);
      if(isJPEGFileName(buf)){
        char path[512];
        sprintf(path,"%s%s",FILEPATH,buf);
        fputs(path,stdout);
        printf("\tid: %d hash:%d\n",rank,getHashcodeFromJPEGFile(path));
      }else if(isPNGFileName(buf)){
        char path[512];
        sprintf(path,"%s%s",FILEPATH,buf);
        fputs(path,stdout);
        //printf("\tid: %d hash:%d\n",rank,getHashcodeFromPNGFile(path));
        printf("暂未实现png文件读取：\n");
      }
    }
  }
  return 0;
}
