#pragma once
static const char LOCAL_FRAME_HTML[] PROGMEM = R"FRAME(<!doctype html>
<html lang="zh"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>InkSight 离线画布</title><style>
body{max-width:42rem;margin:1rem auto;padding:0 1rem;font:16px system-ui;color:#302b25}
canvas{width:100%;border:1px solid #888}label,button,textarea{display:block;margin:.7rem 0}textarea{width:95%}button{padding:.6rem}
</style><h1>离线画布</h1><p>照片、文字和四色预览只在手机上处理。发送后设备将刷屏并休眠。</p>
<input id="file" type="file" accept="image/*"><label>构图 <select id="fit"><option value="contain">完整显示</option><option value="cover">居中裁满</option></select></label>
<label>照片色调 <select id="tone"><option value="adaptive">eink-warm · 全面暖色</option><option value="natural">eink-warm · 自然暖调</option><option value="off">原色（关闭暖调）</option></select></label>
<label>便签或照片题字<textarea id="note" rows="3" maxlength="300" placeholder="每行一段，可只写文字"></textarea></label>
<label>字色 <select id="ink"><option>#000000</option><option>#c80000</option><option>#e8b000</option><option>#ffffff</option></select></label>
<canvas id="view" width="768" height="552"></canvas><button id="send" disabled>发送、刷屏并休眠</button><p id="status" role="status"></p>
<script>
const $=id=>document.getElementById(id), canvas=$('view'),cx=canvas.getContext('2d'),w=canvas.width,h=canvas.height;
const palette=[[0,0,0],[255,255,255],[232,176,0],[200,0,0]], kernel=[[1,0],[2,0],[-1,1],[0,1],[1,1],[0,2]];
let photo=null,frame=null,sending=false,loadId=0;
// Offline counterpart of backend/core/eink_warm.py. Both output rounded RGB
// before Atkinson; text is applied separately using exact palette indices.
const clip=(x,lo,hi)=>Math.max(lo,Math.min(hi,x));
const luma=(r,g,b)=>.2126*r+.7152*g+.0722*b;
const sunset=(()=>{
 const stops=[[0,0,0,0],[.10,26,6,4],[.26,92,18,12],[.42,168,40,16],[.56,214,86,18],
              [.70,236,140,26],[.84,248,200,60],[.93,255,235,150],[1,255,255,255]];
 return Array.from({length:256},(_,i)=>{
   const t=i/255;let s=1;while(stops[s][0]<t)s++;
   const a=stops[s-1],b=stops[s],f=(t-a[0])/(b[0]-a[0]);
   return [1,2,3].map(c=>a[c]+f*(b[c]-a[c]));
 });
})();
function warmPhoto(rgb,width,height,preset){
 const n=width*height,hist=new Uint32Array(256);
 for(let j=0;j<rgb.length;j+=3)hist[Math.round(luma(rgb[j],rgb[j+1],rgb[j+2]))]++;
 function percentile(p){let count=0;for(let i=0;i<256;i++){count+=hist[i];if(count>=n*p)return i;}}
 const lo=percentile(.005),hi=percentile(.995);
 if(hi-lo>=8)for(let j=0;j<rgb.length;j++)rgb[j]=clip(18+(rgb[j]-lo)*228/(hi-lo),0,255);
 const stride=width*3,blur=new Float64Array(rgb.length);
 for(let j=0;j<rgb.length;j++)blur[j]=(rgb[Math.max(j-stride,j%stride)]+2*rgb[j]+
   rgb[Math.min(j+stride,rgb.length-stride+j%stride)])/4;
 for(let y=0;y<height;y++)for(let x=0;x<width;x++){
   const j=y*stride+3*x,left=x?j-3:j,right=x+1<width?j+3:j;
   for(let c=0;c<3;c++){const soft=(blur[left+c]+2*blur[j+c]+blur[right+c])/4;
     rgb[j+c]=clip(rgb[j+c]+.30*(rgb[j+c]-soft),0,255)/255;}
   const r=rgb[j],g=rgb[j+1],b=rgb[j+2],mx=Math.max(r,g,b),mn=Math.min(r,g,b);
   const light=(mx+mn)/2,chroma=mx-mn;
   let hue=chroma?((60*(mx===r?(g-b)/chroma:mx===g?2+(b-r)/chroma:4+(r-g)/chroma))%360+360)%360:0;
   let sat=clip(chroma/Math.max(1e-12,1-Math.abs(2*light-1)),0,1);
   if(preset==='natural'){
     const t=Math.max(clip((110-hue)/50,0,1),clip((hue-300)/50,0,1)),a=t*t*(3-2*t);
     sat=sat**(1-.45*a)*(.75+.25*a);
     hue+=clip(-.25*(1-a)*(hue-60),-60,0)+a*clip(.12*((570-hue)%360-180),-18,18);
   }else hue=1+hue*58/360;
   const amp=sat*Math.min(light,1-light);
   for(let c=0;c<3;c++){const phase=(hue/30+(c===0?0:c===1?8:4)+12)%12;
     rgb[j+c]=255*(light-amp*clip(Math.min(phase-3,9-phase),-1,1));}
   if(preset==='adaptive'){const ramp=sunset[clip(Math.round(luma(rgb[j],rgb[j+1],rgb[j+2])),0,255)];
     for(let c=0;c<3;c++)rgb[j+c]=.6*rgb[j+c]+.4*ramp[c];}
   for(let c=0;c<3;c++)rgb[j+c]=Math.round(clip(rgb[j+c],0,255));
 }
}
function draw(){
 cx.fillStyle='white';cx.fillRect(0,0,w,h);
 if(photo){const k=Math[$('fit').value==='cover'?'max':'min'](w/photo.width,h/photo.height);
 cx.drawImage(photo,(w-k*photo.width)/2,(h-k*photo.height)/2,k*photo.width,k*photo.height);}
 const image=cx.getImageData(0,0,w,h),p=image.data,buf=new Float64Array(w*h*3);
 for(let i=0;i<w*h;i++){
   buf[3*i]=p[4*i];buf[3*i+1]=p[4*i+1];buf[3*i+2]=p[4*i+2];
 }
 if(photo&&$('tone').value!=='off')warmPhoto(buf,w,h,$('tone').value);
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
