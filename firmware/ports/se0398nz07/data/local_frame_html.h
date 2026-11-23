#pragma once
static const char LOCAL_FRAME_HTML[] PROGMEM = R"FRAME(<!doctype html>
<html lang="zh"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>InkSight 离线画布</title><style>
body{max-width:42rem;margin:1rem auto;padding:0 1rem;font:16px system-ui;color:#302b25}
canvas{width:100%;border:1px solid #888}label,button,textarea{display:block;margin:.7rem 0}textarea{width:95%}button{padding:.6rem}
</style><h1>离线画布</h1><p>照片、文字和四色预览只在手机上处理。发送后设备将刷屏并休眠。</p>
<input id="file" type="file" accept="image/*"><label>构图 <select id="fit"><option value="contain">完整显示</option><option value="cover">居中裁满</option></select></label>
<label>色调 <select id="tone"><option value="warm">暖色映射</option><option value="original">原色</option></select></label>
<label>便签或照片题字<textarea id="note" rows="3" maxlength="300" placeholder="每行一段，可只写文字"></textarea></label>
<label>字色 <select id="ink"><option>#000000</option><option>#c80000</option><option>#e8b000</option><option>#ffffff</option></select></label>
<canvas id="view" width="768" height="552"></canvas><button id="send" disabled>发送、刷屏并休眠</button><p id="status" role="status"></p>
<script>
const $=id=>document.getElementById(id), canvas=$('view'),cx=canvas.getContext('2d'),w=canvas.width,h=canvas.height;
const palette=[[0,0,0],[255,255,255],[232,176,0],[200,0,0]], kernel=[[1,0],[2,0],[-1,1],[0,1],[1,1],[0,2]];
let photo=null,frame=null,sending=false,loadId=0;
function draw(){
 cx.fillStyle='white';cx.fillRect(0,0,w,h);
 if(photo){const k=Math[$('fit').value==='cover'?'max':'min'](w/photo.width,h/photo.height);
 cx.drawImage(photo,(w-k*photo.width)/2,(h-k*photo.height)/2,k*photo.width,k*photo.height);}
 const image=cx.getImageData(0,0,w,h),p=image.data,buf=new Float32Array(w*h*3);
 for(let i=0;i<w*h;i++){
   let r=p[4*i],g=p[4*i+1],b=p[4*i+2];
   if(photo&&$('tone').value==='warm'){const y=.2126*r+.7152*g+.0722*b,c=Math.max(r,g,b)-Math.min(r,g,b);
     r=y+.72*c;g=y-.08*c;b=y-.62*c;}
   buf.set([r,g,b],3*i);
 }
 frame=new Uint8Array(w*h/4);
 for(let y=0;y<h;y++)for(let x=0;x<w;x++){
   const i=y*w+x,j=i*3;let best=0,dist=Infinity;
   for(let n=0;n<4;n++){const c=palette[n],d=(buf[j]-c[0])**2+(buf[j+1]-c[1])**2+(buf[j+2]-c[2])**2;
     if(d<dist){dist=d;best=n;}}
   frame[i>>2]|=best<<(6-2*(i%4));
   for(let c=0;c<3;c++){const v=palette[best][c],e=(buf[j+c]-v)/8;p[4*i+c]=v;
     for(const [dx,dy] of kernel){const xx=x+dx,yy=y+dy;if(xx>=0&&xx<w&&yy<h){const a=3*(yy*w+xx)+c;buf[a]=Math.max(0,Math.min(255,buf[a]+e));}}}
 }
 cx.putImageData(image,0,0);
 // Draw text after dithering so text and its preview use exact ink colors.
 const lines=$('note').value.split('\n').slice(0,12);
 if($('note').value){const layer=document.createElement('canvas');layer.width=w;layer.height=h;
   const t=layer.getContext('2d');t.font='bold 34px sans-serif';
   lines.forEach((s,i)=>t.fillText(s,18,h-20-(lines.length-1-i)*42,w-36));
   const mask=t.getImageData(0,0,w,h).data,ink=[0,3,2,1][$('ink').selectedIndex];
   for(let i=0;i<w*h;i++)if(mask[4*i+3]>=128){const shift=6-2*(i%4);frame[i>>2]=(frame[i>>2]&~(3<<shift))|(ink<<shift);p.set(palette[ink],4*i);}
   cx.putImageData(image,0,0);}
 $('send').disabled=sending||(!photo&&!$('note').value.trim());
}
let timer;function schedule(){clearTimeout(timer);$('send').disabled=true;timer=setTimeout(draw,120);}
for(const id of ['fit','tone','ink','note'])$(id).oninput=schedule;
$('file').onchange=async()=>{const id=++loadId,f=$('file').files[0];photo=null;draw();if(!f)return;
 const url=URL.createObjectURL(f);try{const im=new Image();im.src=url;await im.decode();if(id===loadId){photo=im;draw();}}
 catch(e){$('status').textContent='无法读取照片，请选择 JPEG 或 PNG。';}finally{URL.revokeObjectURL(url);}};
$('send').onclick=async()=>{if(sending||!frame)return;sending=true;$('send').disabled=true;
 try{let crc=-1;for(const b of frame){crc^=b;for(let j=0;j<8;j++)crc=(crc>>>1)^((crc&1)?0xedb88320:0);}
 const form=new FormData();form.append('frame',new Blob([frame]),((~crc)>>>0).toString(16).padStart(8,'0')+'.bin');$('status').textContent='正在发送…';
 const res=await fetch('/local-frame',{method:'POST',body:form});
 if(!res.ok)throw Error('HTTP '+res.status);$('status').textContent='设备已收齐，正在刷屏。完成后热点会关闭。';}
 catch(e){sending=false;$('send').disabled=false;$('status').textContent='发送失败：'+e.message+'。请确认仍连接设备热点。';}};
draw();</script></html>)FRAME";
