/*
 * Zaks Wang
 * 图像处理帮助类
 * 2013-2-18
 */
#ifndef imagHelper_h
#define imagHelper_h
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>
typedef enum {false = 0, true = 1} bool;
/*
 * 定义图像的数据类型
 */
typedef struct image{
  unsigned short width; /*图像宽度*/
  unsigned short high; /*图像高度*/
  unsigned char * ptr_img_buf; /*像素数组*/
  unsigned short ele; /*每个像素的元素，默认为3（rgb），有可能为4表示（rgba）*/
} _image;

/*
   int rgbToGray_pixel(const int pixels) {
//int _alpha = (pixels >> 24) & 0xFF;
char red =(char)((pixels >> 16) & 0xFF);
char green = (char)((pixels >> 8) & 0xFF);
char blue = (char)((pixels) & 0xFF);
return (int) (0.3 * red + 0.59 * green + 0.11 * blue);
}
*/
/*
 * 读取指定文件名的jpeg文件到_image数据结构
 */

_image *read_JPEG_file (const char * filename){
  _image *pImag = (_image*)malloc(sizeof(_image));
  pImag->ptr_img_buf = NULL;
  pImag->ele = pImag->high = pImag->width = 0;
  struct jpeg_decompress_struct cinfo;
  //struct my_error_mgr jerr;
  struct jpeg_error_mgr jerr;
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/*行缓冲*/

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "无法打开%s文件\n", filename);
    return NULL;
  }

  cinfo.err = jpeg_std_error(&jerr); //采用系统的错误处理
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  pImag->high = cinfo.output_height;
  pImag->width = cinfo.output_width;
  pImag->ele = cinfo.output_components;
  pImag->ptr_img_buf = (unsigned char*)malloc(sizeof(unsigned char)*pImag->width*
        pImag->high*pImag->ele);
  if(pImag->ptr_img_buf==NULL){
    fprintf(stderr,"分配%s图片内存出错",filename);
    free(pImag);
    return NULL;
  }
  unsigned char* pCur = pImag->ptr_img_buf;
  buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, pImag->width*pImag->ele, 1);
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    memcpy(pCur,buffer[0],pImag->width*pImag->ele);
    pCur+=(pImag->width*pImag->ele);
  }
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  return pImag;
}
/*
 * 读取PNG图片
 * bug:错误的读取方法
 */
_image * read_PNG_file(const char * filename){
  _image *pImag = (_image*)malloc(sizeof(_image));
  pImag->ptr_img_buf = NULL;
  pImag->width = pImag->high = pImag->ele = 0;
  png_image image;
  memset(&image,0,sizeof(image));
  image.version -= PNG_IMAGE_VERSION;
  if (png_image_begin_read_from_file(&image, filename))
  {
    png_bytep buffer;
    image.format = PNG_FORMAT_RGBA;
    buffer = malloc(PNG_IMAGE_SIZE(image));
    if(buffer != NULL){
      if(png_image_finish_read(&image,NULL,buffer,0,NULL)){
        //读取完成
        pImag->ele = 4;
        pImag->width = image.width;
        pImag->high -= image.height;
        pImag->ptr_img_buf = (unsigned char *) buffer;
      }else{
        fprintf(stderr,"读取%s图片出错\n",filename);
        free(pImag);
        free(buffer);
        return NULL;
      }
    }else{
      fprintf(stderr,"分配%s图片内存出错\n",filename);
      free(pImag);
      return NULL;
    }
  }else{
    fprintf(stderr,"读取%s图片头出错\n",filename);
    free(pImag);
    return NULL;
  }
  return pImag;
}

/*
 * 将指向r开头的3个值改为灰度
 */
void rgbToGray_pixel(unsigned char *pd){
  *pd=0.3*(*pd); /*R*/
  *(pd+1)=0.59*(*(pd+1)); /*G*/
  *(pd+2)=0.11*(*(pd+2)); /*B*/
}
/*
 * 将图像RGB转换为灰度值
 */
void rgbToGray_img(_image *img){
  unsigned short i,j;
  for(i=0;i<img->high;i++)
  for(j=0;j<img->width;j++){
    rgbToGray_pixel(&(img->ptr_img_buf[i*img->width*img->ele+j*img->ele]));
  }
}
/*
 * 将图像缩小到w宽h高
 * 采用最近邻插值法计算
 */
void thumb(_image *img,const unsigned short w,const unsigned short h){
  _image newImag;
  newImag.width = w;
  newImag.high  = h;
  newImag.ele = img->ele;
  newImag.ptr_img_buf = (unsigned char *)malloc(sizeof(unsigned char)*newImag.ele*newImag.width*newImag.high);
  float fw = (float)img->width/w;
  float fh = (float)img->high/h;
  unsigned short *array_x = (unsigned short *)malloc(sizeof(unsigned short)*w);
  unsigned short *array_y = (unsigned short *)malloc(sizeof(unsigned short)*h);
  int i;
  for(i=0;i<h;i++){
    array_y[i]=(int)(i*fh+0.5f);
  }
  for(i=0;i<w;i++){
    array_x[i]=(int)(i*fw+0.5f);
  }
  unsigned char *pS = img->ptr_img_buf,*pD = newImag.ptr_img_buf;
  unsigned char *ptrS,*ptrD;
  int j;
  for(i=0;i<h;i++){
    ptrS = pS + img->ele*img->width*array_y[i];
    ptrD = pD + newImag.ele*w*i;
    for(j=0;j<w;j++){
      memcpy(ptrD+newImag.ele*j,ptrS+array_x[j]*img->ele,img->ele);
    }
  }
  img->width = w;
  img->high = h;
  free(img->ptr_img_buf);
  img->ptr_img_buf = newImag.ptr_img_buf;
  free(array_x);
  free(array_y);
}
/*
 * 根据图像获得哈希值,此处只考虑图像大小小于等于64个像素的情况
 */
long long getHashcode(_image *img){
  long long sum=0;
  unsigned short i,j,count=0;
  long long hashcode = 0;
  int *newImag = (int *)malloc(sizeof(int)*img->high*img->width);
  for(i=0;i<img->high;i++){
    for(j=0;j<img->width;j++){
      unsigned char * p = &(img->ptr_img_buf[i*img->width+j]);
      newImag[i*img->width+j]=(int)(*p)+((int)(*(p+1)))+((int)(*(p+2)));
      sum+=newImag[i*img->width+j];
    }
  }
  sum/=(img->width*img->high);
  for(i=0;i<img->high;i++){
    for(j=0;j<img->width;j++){
      if(newImag[i*img->width+j]>sum){
        hashcode+=(1<<count);
      }
      count++;
    }
  }
  return hashcode;
}


long long getHashcodeFromJPEGFile(const char * filename){
  _image *fileImag = read_JPEG_file(filename);
  if(fileImag==NULL){
    return -1;
  }
  /*step one: 缩小尺寸到8*8规格*/
  thumb(fileImag,8,8);
  /*step two: 将图像转换为灰度图像*/
  rgbToGray_img(fileImag);
  /*step three: 计算哈希*/
  long long hashcode = getHashcode(fileImag);
  free(fileImag->ptr_img_buf);
  free(fileImag);
  return hashcode;
}
long long getHashcodeFromPNGFile(const char* filename){
  _image *fileImag = read_PNG_file(filename);
  if(fileImag==NULL){
    return -1;
  }
  /*step one: 缩小尺寸到8*8规格*/
  thumb(fileImag,8,8);
  /*step two: 将图像转换为灰度图像*/
  rgbToGray_img(fileImag);
  /*step three: 计算哈希*/
  long long hashcode = getHashcode(fileImag);
  free(fileImag->ptr_img_buf);
  free(fileImag);
  return hashcode;
}

/*
int main(){
  _image *fileImag = read_JPEG_file("sample12.jpg");
  if(fileImag==NULL){
  return -1;}

 // step one: 缩小尺寸到8*8规格
  thumb(fileImag,8,8);
 // step two: 将图像转换为灰度图像
  rgbToGray_img(fileImag);
 // step three: 计算哈希
  long long hashcode = getHashcode(fileImag);
  printf("%d\n",hashcode);
  free(fileImag->ptr_img_buf);
  free(fileImag);
  return 0;
}
*/

bool isJPEGFileName(const char * buf){
  unsigned char len = (char)strlen(buf);
  if(len>4&&buf[len-1]=='g'&&buf[len-2]=='p'&&buf[len-3]=='j'&&buf[len-4]=='.'){
    return true;
  }
  return false;
}
bool isPNGFileName(const char * buf){
  unsigned char len = (char)strlen(buf);
  if(len>4&&buf[len-1]=='g'&&buf[len-2]=='n'&&buf[len-3]=='p'&&buf[len-4]=='.'){
    return true;
  }
  return false;
}
#endif
