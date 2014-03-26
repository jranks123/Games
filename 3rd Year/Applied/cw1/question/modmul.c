#include "modmul.h"
#include <gmp.h>


/*
Perform stage 1:

- read each 3-tuple of N, e and m from stdin,
- compute the RSA encryption c,
- then write the ciphertext c to stdout.
*/

void stage1() {
  // fill in this function with solution
  	mpz_t N, e, m, c;
    int lineCount = 0;
    mpz_inits(N,e,m,c,NULL);
    do{
      lineCount = gmp_scanf( "%Zx\n%Zx\n%Zx", N, e, m );
       if(lineCount == 3){
        mpz_powm(c, m, e, N);
        gmp_printf( "%Zx\n", c );
       }
    } while (lineCount == 3 );
    mpz_clear( N );
    mpz_clear( e );
    mpz_clear( m );

}

/*
Perform stage 2:

- read each 9-tuple of N, d, p, q, d_p, d_q, i_p, i_q and c from stdin,
- compute the RSA decryption m,
- then write the plaintext m to stdout.
*/

void stage2() {
    mpz_t N, d, p, q, d_p, d_q, i_p, i_q, c, m, u, temp1, temp2;
    int lineCount = 0;
    mpz_inits(N,d,p,q,d_p,d_q,i_p,i_q,c,m,u,temp1, temp2, NULL);
    do{
      lineCount = gmp_scanf( "%Zx\n%Zx\n%Zx\n%Zx\n%Zx\n%Zx\n%Zx\n%Zx\n%Zx",N,d, p, q, d_p, d_q, i_p, i_q, c);
      if(lineCount == 9){
        mpz_powm(temp1, c, d_q,q);
        mpz_powm(temp2, c, d_p,p);
        
        mpz_sub(u, temp1, temp2);
        mpz_mul(u, u, i_p);
        mpz_mod(u,u,q);

        mpz_mul(m, p, u);
        mpz_add(m, m, temp2);
        mpz_mod(m,m,N);
        //mpz_powm(m, c, d, N);
        gmp_printf( "%Zx\n", m );
      }
    }while (lineCount == 9);
    mpz_clear( N );mpz_clear( d );mpz_clear( p );
    mpz_clear( q );mpz_clear( d_p );mpz_clear( d_q );
    mpz_clear( i_p );mpz_clear( i_q );mpz_clear( c );
    mpz_clear( m );
}

/*
Perform stage 3:

- read each 5-tuple of p, q, g, h and m from stdin,
- compute the ElGamal encryption c = (c_1,c_2),
- then write the ciphertext c to stdout.
*/






void stage3() {

  // fill in this function with solution
    mpz_t p, q, g, h, m, c1, c2, k, seed;
    int lineCount = 0;

    int seedVal = time(NULL);
    gmp_randstate_t randState;

    mpz_inits(p,q,g,h,m,c1,c2,k, seed, NULL);


    mpz_set_ui(seed, seedVal);
    gmp_randinit_default(randState);
    do{

      lineCount  =gmp_scanf( "%Zx\n%Zx\n%Zx\n%Zx\n%Zx",  p,q,g,h,m );
      gmp_randseed (randState, seed);
      mpz_urandomm(k, randState, q);
      if(lineCount == 5){
        mpz_powm(c1, g, k, p);
        mpz_powm(c2, h, k, p);
        mpz_mul(c2, c2, m);
        mpz_mod(c2, c2, p);
        gmp_printf( "%Zx\n", c1 );      
        gmp_printf( "%Zx\n", c2 );
      }

    }while(lineCount == 5);
      mpz_clear( p );mpz_clear( q );mpz_clear( g );
      mpz_clear( h );mpz_clear( m );mpz_clear( c1 );
      mpz_clear( c2 ); mpz_clear( k );mpz_clear( seed );



}

/*
Perform stage 4:

- read each 5-tuple of p, q, g, x and c = (c_1,c_2) from stdin,
- compute the ElGamal decryption m,
- then write the plaintext m to stdout.
*/

void stage4() {

  // fill in this function with solution
    mpz_t p, q, g, h, m, c1, c2;
    int lineCount = 0;
    mpz_inits(p,q,g,h,m,c1,c2, NULL);
    do{
      lineCount  =gmp_scanf( "%Zx\n%Zx\n%Zx\n%Zx\n%Zx\n%Zx",  p,q,g,h,c1,c2 );
      if(lineCount == 6){
        mpz_sub(m, q, h);
        mpz_powm(m, c1, m, p);
        mpz_mul(m, m, c2);   
        mpz_mod(m, m, p);   
        gmp_printf( "%Zx\n", m );
      }
    }while(lineCount == 6);
    mpz_clear( p );mpz_clear( q );mpz_clear( g );
    mpz_clear( h );mpz_clear( m );mpz_clear( c1 );
    mpz_clear( c2 );
}

/*
The main function acts as a driver for the assignment by simply invoking
the correct function for the requested stage.
*/

int main( int argc, char* argv[] ) {
  if     ( !strcmp( argv[ 1 ], "stage1" ) ) {
    stage1();
  }
  else if( !strcmp( argv[ 1 ], "stage2" ) ) {
    stage2();
  }
  else if( !strcmp( argv[ 1 ], "stage3" ) ) {
    stage3();
  }
  else if( !strcmp( argv[ 1 ], "stage4" ) ) {
    stage4();
  }

  return 0;
}
