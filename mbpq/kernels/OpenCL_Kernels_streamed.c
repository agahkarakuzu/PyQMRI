
__kernel void update_v(__global float8 *v,__global float8 *v_, __global float8 *Kyk2, const float tau) {
  size_t i = get_global_id(0);
  v[i] = v_[i]-tau*Kyk2[i];
}

__kernel void update_r(__global float2 *r, __global float2 *r_, __global float2 *A, __global float2 *A_, __global float2 *res,
                          const float sigma, const float theta, const float lambdainv) {
  size_t i = get_global_id(0);

  r[i] = (r_[i]+sigma*((1+theta)*A[i]-theta*A_[i] - res[i]))*lambdainv;
}
__kernel void update_z2(__global float16 *z_new, __global float16 *z, __global float16 *gx,__global float16 *gx_,
                          const float sigma, const float theta, const float alphainv,
                          const int NUk) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;

  float fac = 0.0f;

  for (int uk=0; uk<NUk; uk++)
  {
     z_new[i] = z[i] + sigma*((1+theta)*gx[i]-theta*gx_[i]);

     // reproject
     fac = hypot(fac,hypot(
     hypot(hypot(hypot(z_new[i].s0,z_new[i].s1), hypot(z_new[i].s2,z_new[i].s3)),hypot(z_new[i].s4,z_new[i].s5)),
     hypot(hypot(2.0f*hypot(z_new[i].s6,z_new[i].s7),2.0f*hypot(z_new[i].s8,z_new[i].s9)),2.0f*hypot(z_new[i].sa,z_new[i].sb)))*alphainv);
     i+=Nx*Ny;
   }
  i = k*Nx*Ny*NUk+Nx*y + x;
  for (int uk=0; uk<NUk; uk++)
  {
    if (fac > 1.0f) {z_new[i] /=fac;}
    i+=Nx*Ny;
  }
}
__kernel void update_z1(__global float8 *z_new, __global float8 *z, __global float8 *gx,__global float8 *gx_,
                          __global float8 *vx, __global float8 *vx_, const float sigma, const float theta, const float alphainv,
                          const int NUk_tgv, const int NUk_H1, const float h1inv) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk_tgv+Nx*y + x;

  float fac = 0.0f;

  for (int uk=0; uk<NUk_tgv; uk++)
  {
     z_new[i] = z[i] + sigma*((1+theta)*gx[i]-theta*gx_[i]-(1+theta)*vx[i]+theta*vx_[i]);

     // reproject
     fac = hypot(fac,hypot(hypot(z_new[i].s0,z_new[i].s1), hypot(hypot(z_new[i].s2,z_new[i].s3),hypot(z_new[i].s4,z_new[i].s5)))*alphainv);
     i+=Nx*Ny;
  }
  i = k*Nx*Ny*NUk_tgv+Nx*y + x;
  for (int uk=0; uk<NUk_tgv; uk++)
  {
    if (fac > 1.0f) {z_new[i] /=fac;}
    i+=Nx*Ny;
  }
  i = k*Nx*Ny*NUk_tgv + Nx*Ny*NUk_tgv + Nx*y + x;
  for (int uk=NUk_tgv; uk<(NUk_tgv+NUk_H1); uk++)
  {
    z_new[i] = (z[i] + sigma*((1+theta)*gx[i]-theta*gx_[i]))*h1inv;
    i += Nx*Ny;
  }
}
  __kernel void update_z1_tv(__global float8 *z_new, __global float8 *z, __global float8 *gx,__global float8 *gx_,
                          const float sigma, const float theta, const float alphainv,
                          const int NUk_tgv, const int NUk_H1, const float h1inv) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk_tgv+Nx*y + x;

  float fac = 0.0f;

  for (int uk=0; uk<NUk_tgv; uk++)
  {
     z_new[i] = z[i] + sigma*((1+theta)*gx[i]-theta*gx_[i]);

     // reproject
     fac = hypot(fac,hypot(hypot(z_new[i].s0,z_new[i].s1), hypot(hypot(z_new[i].s2,z_new[i].s3),hypot(z_new[i].s4,z_new[i].s5)))*alphainv);
     i+=Nx*Ny;
  }
  i = k*Nx*Ny*NUk_tgv+Nx*y + x;
  for (int uk=0; uk<NUk_tgv; uk++)
  {
    if (fac > 1.0f) z_new[i] /=fac;
    i+=Nx*Ny;
  }
  i = k*Nx*Ny*NUk_tgv + Nx*Ny*NUk_tgv + Nx*y + x;
  for (int uk=NUk_tgv; uk<(NUk_tgv+NUk_H1); uk++)
  {
    z_new[i] = (z[i] + sigma*((1+theta)*gx[i]-theta*gx_[i]))*h1inv;
    i += Nx*Ny;
  }
}
__kernel void update_primal(__global float2 *u_new, __global float2 *u, __global float2 *Kyk,__global float2 *u_k, const float tau, const float tauinv, float div, __global float* min, __global float* max, __global int* real, const int NUk) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;
  float norm = 0;

  for (int uk=0; uk<NUk; uk++)
  {
     u_new[i] = (u[i]-tau*Kyk[i]+tauinv*u_k[i])*div;

     if(real[uk]>0)
     {
         u_new[i].s1 = 0;
         if (u_new[i].s0<min[uk])
         {
             u_new[i].s0 = min[uk];
         }
         if(u_new[i].s0>max[uk])
         {
             u_new[i].s0 = max[uk];
         }
     }
     else
     {
         norm =  sqrt(pow((float)(u_new[i].s0),(float)(2.0))+pow((float)(u_new[i].s1),(float)(2.0)));
         if (norm<min[uk])
         {
             u_new[i].s0 *= 1/norm*min[uk];
             u_new[i].s1 *= 1/norm*min[uk];
         }
         if(norm>max[uk])
         {
            u_new[i].s0 *= 1/norm*max[uk];
            u_new[i].s1 *= 1/norm*max[uk];
         }
     }
     i+=Nx*Ny;
  }
}

__kernel void gradient(__global float8 *grad, __global float2 *u, const int NUk, __global float* ratio, const float dz) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;


  for (int uk=0; uk<NUk; uk++)
  {
     // gradient
     grad[i] = (float8)(-u[i],-u[i],-u[i]/dz,0.0f,0.0f);
     if (x < Nx-1)
     { grad[i].s01 += u[i+1].s01;}
     else
     { grad[i].s01 = 0.0f;}

     if (y < Ny-1)
     { grad[i].s23 += u[i+Nx].s01;}
     else
     { grad[i].s23 = 0.0f;}
     if (k < NSl-1)
     { grad[i].s45 += u[i+Nx*Ny*NUk].s01/dz;}
     else
     { grad[i].s45 = 0.0f;}
     {grad[i]*=ratio[uk];}
     i+=Nx*Ny;
  }
}

__kernel void sym_grad(__global float16 *sym, __global float8 *w, const int NUk, const float dz) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;


  for (int uk=0; uk<NUk; uk++)
  {
     // symmetrized gradient
     float16 val_real = (float16)(w[i].s024, w[i].s024, w[i].s024,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
     float16 val_imag = (float16)(w[i].s135, w[i].s135, w[i].s135,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
     if (x > 0)
     { val_real.s012 -= w[i-1].s024;  val_imag.s012 -= w[i-1].s135;}
     else
     { val_real.s012 = (float3) 0.0f; val_imag.s012 = (float3) 0.0f; }

     if (y > 0)
     {val_real.s345 -= w[i-Nx].s024;  val_imag.s345 -= w[i-Nx].s135;}
     else
     {val_real.s345 = (float3)  0.0f; val_imag.s345 = (float3) 0.0f;  }

     if (k > 0)
     {val_real.s678 -= w[i-Nx*Ny*NUk].s024;  val_imag.s678 -= w[i-Nx*Ny*NUk].s135;}
     else
     {val_real.s678 = (float3) 0.0f; val_imag.s678 = (float3) 0.0f;  }

     sym[i] = (float16)(val_real.s0, val_imag.s0, val_real.s4,val_imag.s4,val_real.s8/dz,val_imag.s8/dz,
                        0.5f*(val_real.s1 + val_real.s3), 0.5f*(val_imag.s1 + val_imag.s3),
                        0.5f*(val_real.s2 + val_real.s6/dz), 0.5f*(val_imag.s2 + val_imag.s6/dz),
                        0.5f*(val_real.s5 + val_real.s7/dz), 0.5f*(val_imag.s5 + val_imag.s7/dz),
                        0.0f,0.0f,0.0f,0.0f);
    i+=Nx*Ny;
   }
}
__kernel void divergence(__global float2 *div, __global float8 *p, const int NUk,
                         __global float* ratio, const int last, const float dz) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;

  for (int ukn=0; ukn<NUk; ukn++)
  {
     // divergence
     float8 val = p[i];
     if (x == Nx-1)
     {
         //real
         val.s0 = 0.0f;
         //imag
         val.s1 = 0.0f;
     }
     if (x > 0)
     {
         //real
         val.s0 -= p[i-1].s0;
         //imag
         val.s1 -= p[i-1].s1;
     }
     if (y == Ny-1)
     {
         //real
         val.s2 = 0.0f;
         //imag
         val.s3 = 0.0f;
     }
     if (y > 0)
     {
         //real
         val.s2 -= p[i-Nx].s2;
         //imag
         val.s3 -= p[i-Nx].s3;
     }
    if (last == 1)
     { if (k == NSl-1){
         //real
         val.s4 = 0.0f;
         //imag
         val.s5 = 0.0f;}
     }
     if (k > 0)
     {
         //real
         val.s4 -= p[i-Nx*Ny*NUk].s4;
         //imag
         val.s5 -= p[i-Nx*Ny*NUk].s5;
     }
     div[i] = val.s01+val.s23+val.s45/dz;
     {div[i]*=ratio[ukn];}
     i+=Nx*Ny;
  }

}
__kernel void sym_divergence(__global float8 *w, __global float16 *q,
                       const int NUk, const int first, const float dz) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;

  for (int uk=0; uk<NUk; uk++)
  {
     // divergence
     float16 val0 = -q[i];
     float16 val_real = (float16)(val0.s0, val0.s6, val0.s8,
                                  val0.s6, val0.s2, val0.sa,
                                  val0.s8, val0.sa, val0.s4,
                                  0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
     float16 val_imag = (float16)(val0.s1, val0.s7, val0.s9,
                                  val0.s7, val0.s3, val0.sb,
                                  val0.s9, val0.sb, val0.s5,
                                  0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
     if (x == 0)
     {
         //real
         val_real.s012 = 0.0f;
         //imag
         val_imag.s012 = 0.0f;
     }
     if (x < Nx-1)
     {
         //real
         val_real.s012 += (float3)(q[i+1].s0, q[i+1].s68);
         //imag
         val_imag.s012 += (float3)(q[i+1].s1, q[i+1].s79);
     }
     if (y == 0)
     {
         //real
         val_real.s345 = 0.0f;
         //imag
         val_imag.s345 = 0.0f;
     }
     if (y < Ny-1)
     {
         //real
         val_real.s345 += (float3)(q[i+Nx].s6, q[i+Nx].s2, q[i+Nx].sa);
         //imag
         val_imag.s345 += (float3)(q[i+Nx].s7, q[i+Nx].s3, q[i+Nx].sb);
     }
     if (first == 1)
     {if (k == 0)
     {
         //real
         val_real.s678 = 0.0f;
         //imag
         val_imag.s678 = 0.0f;
         }
     }
     if (k < NSl-1)
     {
         //real
         val_real.s678 += (float3)(q[i+Nx*Ny*NUk].s8a, q[i+Nx*Ny*NUk].s4);
         //imag
         val_imag.s678 += (float3)(q[i+Nx*Ny*NUk].s9b, q[i+Nx*Ny*NUk].s5);
     }
     // linear step
     //real
     w[i].s024 = val_real.s012 + val_real.s345 + val_real.s678/dz;
     //imag
     w[i].s135 = val_imag.s012 + val_imag.s345 + val_imag.s678/dz;
     i+=Nx*Ny;
  }
}
__kernel void update_Kyk2(__global float8 *w, __global float16 *q, __global float8 *z,
                       const int NUk, const int first, const float dz) {
  size_t Nx = get_global_size(2), Ny = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2), y = get_global_id(1);
  size_t k = get_global_id(0);
  size_t i = k*Nx*Ny*NUk+Nx*y + x;

  for (int uk=0; uk<NUk; uk++)
  {
     // divergence
     float16 val0 = -q[i];
     float16 val_real = (float16)(val0.s0, val0.s6, val0.s8,
                                  val0.s6, val0.s2, val0.sa,
                                  val0.s8, val0.sa, val0.s4,
                                  0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
     float16 val_imag = (float16)(val0.s1, val0.s7, val0.s9,
                                  val0.s7, val0.s3, val0.sb,
                                  val0.s9, val0.sb, val0.s5,
                                  0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
     if (x == 0)
     {
         //real
         val_real.s012 = 0.0f;
         //imag
         val_imag.s012 = 0.0f;
     }
     if (x < Nx-1)
     {
         //real
         val_real.s012 += (float3)(q[i+1].s0, q[i+1].s68);
         //imag
         val_imag.s012 += (float3)(q[i+1].s1, q[i+1].s79);
     }
     if (y == 0)
     {
         //real
         val_real.s345 = 0.0f;
         //imag
         val_imag.s345 = 0.0f;
     }
     if (y < Ny-1)
     {
         //real
         val_real.s345 += (float3)(q[i+Nx].s6, q[i+Nx].s2, q[i+Nx].sa);
         //imag
         val_imag.s345 += (float3)(q[i+Nx].s7, q[i+Nx].s3, q[i+Nx].sb);
     }
     if (first == 1)
     {
          if (k == 0)
          {
         //real
         val_real.s678 = 0.0f;
         //imag
         val_imag.s678 = 0.0f;
         }
     }
     if (k < NSl-1)
     {
         //real
         val_real.s678 += (float3)(q[i+Nx*Ny*NUk].s8a, q[i+Nx*Ny*NUk].s4);
         //imag
         val_imag.s678 += (float3)(q[i+Nx*Ny*NUk].s9b, q[i+Nx*Ny*NUk].s5);
     }
     // linear step
     //real
     w[i].s024 = -val_real.s012 - val_real.s345 - val_real.s678/dz -z[i].s024;
     //imag
     w[i].s135 = -val_imag.s012 - val_imag.s345 - val_imag.s678/dz -z[i].s135;
     i+=Nx*Ny;
  }
}

__kernel void operator_fwd(__global float2 *out, __global float2 *in,
                       __global float2 *coils, __global float2 *grad, const int NCo,
                       const int NScan, const int Nuk)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);

  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);

  float2 tmp_in = 0.0f;
  float2 tmp_grad = 0.0f;
  float2 tmp_coil = 0.0f;
  float2 tmp_mul = 0.0f;


    for (int scan=0; scan<NScan; scan++)
    {
      for (int coil=0; coil < NCo; coil++)
      {
        tmp_coil = coils[k*NCo*X*Y + coil*X*Y + y*X + x];
        float2 sum = 0.0f;
        for (int uk=0; uk<Nuk; uk++)
        {
          tmp_grad = grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x];
          tmp_in = in[k*Nuk*X*Y+uk*X*Y+y*X+x];

          tmp_mul = (float2)(tmp_in.x*tmp_grad.x-tmp_in.y*tmp_grad.y,tmp_in.x*tmp_grad.y+tmp_in.y*tmp_grad.x);
          sum += (float2)(tmp_mul.x*tmp_coil.x-tmp_mul.y*tmp_coil.y,
                                                    tmp_mul.x*tmp_coil.y+tmp_mul.y*tmp_coil.x);

        }
        out[k*NScan*NCo*X*Y+scan*NCo*X*Y+coil*X*Y + y*X + x] = sum;
      }
    }


}
__kernel void operator_ad(__global float2 *out, __global float2 *in,
                       __global float2 *coils, __global float2 *grad, const int NCo,
                       const int NScan, const int Nuk)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);

  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);


  float2 tmp_in = 0.0f;
  float2 tmp_mul = 0.0f;
  float2 conj_grad = 0.0f;
  float2 conj_coils = 0.0f;


  for (int uk=0; uk<Nuk; uk++)
  {
  float2 sum = (float2) 0.0f;
  for (int scan=0; scan<NScan; scan++)
  {
    conj_grad = (float2) (grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].x,
                          -grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].y);
  for (int coil=0; coil < NCo; coil++)
  {
    conj_coils = (float2) (coils[k*NCo*X*Y + coil*X*Y + y*X + x].x,
                                  -coils[k*NCo*X*Y + coil*X*Y + y*X + x].y);

    tmp_in = in[k*NScan*NCo*X*Y+scan*NCo*X*Y+coil*X*Y + y*X + x];
    tmp_mul = (float2)(tmp_in.x*conj_grad.x-tmp_in.y*conj_grad.y,tmp_in.x*conj_grad.y+tmp_in.y*conj_grad.x);


    sum += (float2)(tmp_mul.x*conj_coils.x-tmp_mul.y*conj_coils.y,
                                     tmp_mul.x*conj_coils.y+tmp_mul.y*conj_coils.x);
  }
  }
  out[k*Nuk*X*Y+uk*X*Y+y*X+x] = sum;
  }

}



__kernel void update_Kyk1(__global float2 *out, __global float2 *in,
                       __global float2 *coils, __global float2 *grad, __global float8 *p, const int NCo,
                       const int NScan, __global float* ratio, const int Nuk, const int last, const float dz)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);

  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);

  size_t i = k*X*Y*Nuk+X*y + x;

  float2 tmp_in = 0.0f;
  float2 tmp_mul = 0.0f;
  float2 conj_grad = 0.0f;
  float2 conj_coils = 0.0f;



  for (int uk=0; uk<Nuk; uk++)
  {

  float2 sum = (float2) 0.0f;
  for (int scan=0; scan<NScan; scan++)
  {
    conj_grad = (float2) (grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].x,
                          -grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].y);
  for (int coil=0; coil < NCo; coil++)
  {
    conj_coils = (float2) (coils[k*NCo*X*Y + coil*X*Y + y*X + x].x,
                                  -coils[k*NCo*X*Y + coil*X*Y + y*X + x].y);

    tmp_in = in[k*NScan*NCo*X*Y+scan*NCo*X*Y+coil*X*Y + y*X + x];
    tmp_mul = (float2)(tmp_in.x*conj_grad.x-tmp_in.y*conj_grad.y,tmp_in.x*conj_grad.y+tmp_in.y*conj_grad.x);


    sum += (float2)(tmp_mul.x*conj_coils.x-tmp_mul.y*conj_coils.y,
                                     tmp_mul.x*conj_coils.y+tmp_mul.y*conj_coils.x);
  }
  }

   // divergence
   float8 val = p[i];
   if (x == X-1)
   {
       //real
       val.s0 = 0.0f;
       //imag
       val.s1 = 0.0f;
   }
   if (x > 0)
   {
       //real
       val.s0 -= p[i-1].s0;
       //imag
       val.s1 -= p[i-1].s1;
   }
   if (y == Y-1)
   {
       //real
       val.s2 = 0.0f;
       //imag
       val.s3 = 0.0f;
   }
   if (y > 0)
   {
       //real
       val.s2 -= p[i-X].s2;
       //imag
       val.s3 -= p[i-X].s3;
   }
   if (last == 1)
   {
    if (k == NSl-1)
    {  //real
       val.s4 = 0.0f;
       //imag
       val.s5 = 0.0f;
    }
   }
   if (k > 0)
   {
       //real
       val.s4 -= p[i-X*Y*Nuk].s4;
       //imag
       val.s5 -= p[i-X*Y*Nuk].s5;
   }
   // scale gradients
   {val*=ratio[uk];}

  out[i] = sum - (val.s01+val.s23+val.s45/dz);
  i+=X*Y;

  }

}

__kernel void update_Kyk1SMS(__global float2 *out, __global float2 *in,
                       __global float8 *p, __global float* ratio, const int Nuk, const int last, const float dz)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);

  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);

  size_t i = k*X*Y*Nuk+X*y + x;

  for (int uk=0; uk<Nuk; uk++)
  {
   // divergence
   float8 val = p[i];
   if (x == X-1)
   {
       //real
       val.s0 = 0.0f;
       //imag
       val.s1 = 0.0f;
   }
   if (x > 0)
   {
       //real
       val.s0 -= p[i-1].s0;
       //imag
       val.s1 -= p[i-1].s1;
   }
   if (y == Y-1)
   {
       //real
       val.s2 = 0.0f;
       //imag
       val.s3 = 0.0f;
   }
   if (y > 0)
   {
       //real
       val.s2 -= p[i-X].s2;
       //imag
       val.s3 -= p[i-X].s3;
   }
   if (last == 1)
   {
    if (k == NSl-1)
    {  //real
       val.s4 = 0.0f;
       //imag
       val.s5 = 0.0f;
    }
   }
   if (k > 0)
   {
       //real
       val.s4 -= p[i-X*Y*Nuk].s4;
       //imag
       val.s5 -= p[i-X*Y*Nuk].s5;
   }
   // scale gradients
   {val*=ratio[uk];}

  out[i] = in[i] - (val.s01+val.s23+val.s45/dz);
  i+=X*Y;

  }

}
//__kernel void update_primal_explicit(__global float2 *out, __global float2 *in, __global float2 *Kyk, __global float2 *u_k,
//__global float2* ATd, const float tau, const float delta_inv, const float lambd, __global float* mmin, __global float* mmax, __global int* real, const int NUk, const int NScan) {
//  size_t X = get_global_size(2), Y = get_global_size(1);
//  size_t NSl = get_global_size(0);
//  size_t x = get_global_id(2);
//  size_t y = get_global_id(1);
//  size_t k = get_global_id(0);
//
//  float2 tmp_in = 0.0f;
//  float2 tmp_grad = 0.0f;
//
//    for (int scan=0; scan<NScan; scan++)
//    {
//        float2 sum = 0.0f;
//        for (int uk=0; uk<NUk; uk++)
//        {
//          tmp_grad = grad[k*NUk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x];
//          tmp_in = in[k*NUk*X*Y+uk*X*Y+y*X+x];
//
//          sum += (float2)(tmp_in.x*tmp_grad.x-tmp_in.y*tmp_grad.y,tmp_in.x*tmp_grad.y+tmp_in.y*tmp_grad.x);
//
//        }
//        out[k*NScan*X*Y+scan*X*Y+ y*X + x] = sum;
//    }
//
//
//}

__kernel void operator_fwd_imagespace(__global float2 *out, __global float2 *in, __global float2 *grad, const int NScan, const int Nuk)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);

  float2 tmp_in = 0.0f;
  float2 tmp_grad = 0.0f;

    for (int scan=0; scan<NScan; scan++)
    {
        float2 sum = 0.0f;
        for (int uk=0; uk<Nuk; uk++)
        {
          tmp_grad = grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x];
          tmp_in = in[k*Nuk*X*Y+uk*X*Y+y*X+x];

          sum += (float2)(tmp_in.x*tmp_grad.x-tmp_in.y*tmp_grad.y,tmp_in.x*tmp_grad.y+tmp_in.y*tmp_grad.x);

        }
        out[k*NScan*X*Y+scan*X*Y+ y*X + x] = sum;
    }


}
__kernel void operator_ad_imagespace(__global float2 *out, __global float2 *in,
                      __global float2 *grad,
                       const int NScan, const int Nuk)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);


  float2 tmp_in = 0.0f;
  float2 conj_grad = 0.0f;



  for (int uk=0; uk<Nuk; uk++)
  {
  float2 sum = (float2) 0.0f;
  for (int scan=0; scan<NScan; scan++)
  {
    conj_grad = (float2) (grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].x,
                          -grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].y);
    tmp_in = in[k*NScan*X*Y + scan*X*Y+ y*X + x];
    sum += (float2)(tmp_in.x*conj_grad.x-tmp_in.y*conj_grad.y,tmp_in.x*conj_grad.y+tmp_in.y*conj_grad.x);

  }
  out[k*Nuk*X*Y+uk*X*Y+y*X+x] = sum;
  }

}



__kernel void update_Kyk1_imagespace(__global float2 *out, __global float2 *in,
                       __global float2 *grad, __global float8 *p,
                       const int NScan, __global float* ratio, const int Nuk, const int last, const float dz)
{
  size_t X = get_global_size(2);
  size_t Y = get_global_size(1);
  size_t NSl = get_global_size(0);
  size_t x = get_global_id(2);
  size_t y = get_global_id(1);
  size_t k = get_global_id(0);

  size_t i = k*X*Y*Nuk+X*y + x;

  float2 tmp_in = 0.0f;
  float2 conj_grad = 0.0f;


  for (int uk=0; uk<Nuk; uk++)
  {
  float2 sum = (float2) 0.0f;
  for (int scan=0; scan<NScan; scan++)
  {
    conj_grad = (float2) (grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].x,
                          -grad[k*Nuk*NScan*X*Y+uk*NScan*X*Y+scan*X*Y + y*X + x].y);
    tmp_in = in[k*NScan*X*Y+scan*X*Y+ y*X + x];
    sum += (float2)(tmp_in.x*conj_grad.x-tmp_in.y*conj_grad.y,tmp_in.x*conj_grad.y+tmp_in.y*conj_grad.x);
  }

   // divergence
   float8 val = p[i];
   if (x == X-1)
   {
       //real
       val.s0 = 0.0f;
       //imag
       val.s1 = 0.0f;
   }
   if (x > 0)
   {
       //real
       val.s0 -= p[i-1].s0;
       //imag
       val.s1 -= p[i-1].s1;
   }
   if (y == Y-1)
   {
       //real
       val.s2 = 0.0f;
       //imag
       val.s3 = 0.0f;
   }
   if (y > 0)
   {
       //real
       val.s2 -= p[i-X].s2;
       //imag
       val.s3 -= p[i-X].s3;
   }
   if (last == 1)
   {
    if (k == NSl-1)
    {  //real
       val.s4 = 0.0f;
       //imag
       val.s5 = 0.0f;
    }
   }
   if (k > 0)
   {
       //real
       val.s4 -= p[i-X*Y*Nuk].s4;
       //imag
       val.s5 -= p[i-X*Y*Nuk].s5;
   }
   // scale gradients
   {val*=ratio[uk];}

  out[i] = sum - (val.s01+val.s23+val.s45/dz);
  i += X*Y;
  }
}

__kernel void perumtescansslices(__global float2 *out, __global float2 *in)
{
  size_t XY = get_global_size(2);
  size_t NSl = get_global_size(1);
  size_t ScanCoil = get_global_size(0);
  size_t xy = get_global_id(2);
  size_t sl = get_global_id(1);
  size_t sc = get_global_id(0);

  out[xy + XY*sc + XY*ScanCoil*sl] = in[xy + XY*sl + XY*NSl*sc];


}