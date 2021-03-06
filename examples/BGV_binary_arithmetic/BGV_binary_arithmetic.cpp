/* Copyright (C) 2019 IBM Corp.
 * This program is Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. See accompanying LICENSE file.
 */

// This is a sample program for education purposes only.
// It attempts to demonstrate the use of the API for the
// binary arithmetic operations that can be performed.

#include <iostream>
using namespace std;
#include <helib/helib.h>
#include <helib/binaryArith.h>
#include <helib/intraSlot.h>

int main(int argc, char* argv[])
{
  /*  Example of binary arithmetic using the BGV scheme  */

  // First set up parameters.
  //
  // NOTE: The parameters used in this example code are for demonstration only.
  // They were chosen to provide the best performance of execution while
  // providing the context to demonstrate how to use the "Binary Arithmetic
  // APIs". The parameters do not provide the security level that might be
  // required by real use/application scenarios.

  // Plaintext prime modulus.
  long p = 2;
  // Cyclotomic polynomial - defines phi(m).
  long m = 4095;
  // Hensel lifting (default = 1).
  long r = 1;
  // Number of bits of the modulus chain.
  long bits = 500;
  // Number of columns of Key-Switching matrix (typically 2 or 3).
  long c = 2;
  // Factorisation of m required for bootstrapping.
  std::vector<long> mvec = {7, 5, 9, 13};
  // Generating set of Zm* group.
  std::vector<long> gens = {2341, 3277, 911};
  // Orders of the previous generators.
  std::vector<long> ords = {6, 4, 6};

  std::cout << "\n*********************************************************";
  std::cout << "\n*            Basic Location operations                  *";
  std::cout << "\n*            ===============================            *";
  std::cout << "\n*                                                       *";
  std::cout << "\n* This is a sample program for testing purposes only.   *";
  std::cout << "\n* It attempts to demonstrate the use of the API for the *";
  std::cout << "\n* basic location operations that can be performed.      *";
  std::cout << "\n*                                                       *";
  std::cout << "\n*********************************************************";
  std::cout << std::endl;

  std::cout << "Initialising context object..." << std::endl;
  // Initialize the context.
  // This object will hold information about the algebra created from the
  // previously set parameters.
  helib::Context context(m, p, r, gens, ords);

  // Modify the context, adding primes to the modulus chain.
  // This defines the ciphertext space.
  std::cout << "Building modulus chain..." << std::endl;
  buildModChain(context, bits, c, /*willBeBootstrappable=*/true);

  // Make bootstrappable.
  // Modify the context, providing bootstrapping capabilities.
  // Boostrapping has the affect of 'refreshing' a ciphertext back to a higher
  // level so more operations can be performed.
  context.enableBootStrapping(
      helib::convert<NTL::Vec<long>, std::vector<long>>(mvec));

  // Print the context.
  context.zMStar.printout();
  std::cout << std::endl;

  // Print the security level.
  std::cout << "Security: " << context.securityLevel() << std::endl;

  // Secret key management.
  std::cout << "Creating secret key..." << std::endl;
  // Create a secret key associated with the context.
  helib::SecKey secret_key(context);
  // Generate the secret key.
  secret_key.GenSecKey();

  // Generate bootstrapping data.
  secret_key.genRecryptData();

  // Public key management.
  // Set the secret key (upcast: SecKey is a subclass of PubKey).
  const helib::PubKey& public_key = secret_key;

  // Get the EncryptedArray of the context.
  const helib::EncryptedArray& ea = *(context.ea);

  // Build the unpack slot encoding.
  std::vector<helib::zzX> unpackSlotEncoding;
  buildUnpackSlotEncoding(unpackSlotEncoding, ea);

  // Get the number of slot (phi(m)).
  long nslots = ea.size();
  std::cout << "Number of slots: " << nslots << std::endl;

  // Generate three random binary numbers a, b, c.
  // Encrypt them under BGV.
  // Calculate a * b + c with HElib's binary arithmetic functions, then decrypt
  // the result.
  // Next, calculate a + b + c with HElib's binary arithmetic functions, then
  // decrypt the result.
  // Finally, calculate popcnt(a) with HElib's binary arithmetic functions,
  // then decrypt the result.  Note that popcnt, also known as hamming weight
  // or bit summation, returns the count of non-zero bits.

  // Each bit of the binary number is encoded into a single ciphertext. Thus
  // for a 16 bit binary number, we will represent this as an array of 16
  // unique ciphertexts.
  // i.e. b0 = [0] [0] [0] ... [0] [0] [0]        ciphertext for bit 0
  //      b1 = [1] [1] [1] ... [1] [1] [1]        ciphertext for bit 1
  //      b2 = [1] [1] [1] ... [1] [1] [1]        ciphertext for bit 2
  // These 3 ciphertexts represent the 3-bit binary number 110b = 6

  // Note: several numbers can be encoded across the slots of each ciphertext
  // which would result in several parallel slot-wise operations.
  // For simplicity we place the same data into each slot of each ciphertext,
  // printing out only the back of each vector.
  // NB: fifteenOrLess4Four max is 15 bits. Later in the code we pop the MSB.
  long bitSize = 16;
  long outSize = 2 * bitSize;
  long a_data =  20;
  long b_data =  20;
  long c_data =  5;

  std::cout << "Pre-encryption data:" << std::endl;
  std::cout << "a = " << a_data << std::endl;
  std::cout << "b = " << b_data << std::endl;
  //std::cout << "c = " << c_data << std::endl;

  // std::vector<long> arr1(ea.size());
  // std::vector<long> arr2(ea.size());
  // arr1={1,2,3,4,5,6,7,8,9,10};
  // arr2={1,2,3,4,5,6,7,8,9,10};

  // Use a scratch ciphertext to populate vectors.
  helib::Ctxt scratch(public_key);
  std::vector<helib::Ctxt> encrypted_a(bitSize, scratch);
  std::vector<helib::Ctxt> encrypted_b(bitSize, scratch);
  std::vector<helib::Ctxt> encrypted_c(bitSize, scratch);

  //Code for testing EA
  // const long num_summands = 5;
  //  std::vector<long> summands_data;
  // for (long i = 0; i < num_summands; ++i)
  //   summands_data.push_back(NTL::RandomBits_long(bitSize));

  // // Encrypt the individual bits.
  // std::vector<helib::Ctxt> encrypted_sum;
  // std::vector<std::vector<helib::Ctxt>> encrypted_summands(5)(5);

  // // Utility function for encrypting a number into a binary representation.
  // const auto encrypt_binary_number =
  //     [&](const long num) -> std::vector<helib::Ctxt> {
  //   std::vector<helib::Ctxt> encrypted_num;
  //   // Resize the vector of ciphertexts (encrypted_bits) to match the input
  //   // size.
  //   helib::resize(encrypted_num, bitSize, helib::Ctxt(secret_key));
  //   for (long i = 0; i < bitSize; i++) {
  //     secret_key.Encrypt(encrypted_num[i], NTL::ZZX((num >> i) & 1));
  //     // if (bootstrap) { // If bootstrapping then modulo down to a lower level.
  //     //   encrypted_num[i].bringToSet(context.getCtxtPrimes(5));
  //     // }
  //   }
  //   return encrypted_num;
  // };

  // // Encrypt the set of numbers into binary representation.
  // for (long i = 0; i < num_summands; ++i)
  //   encrypted_summands.push_back(encrypt_binary_number(summands_data[i]));
  // for (auto& slot:encrypted_summands)
  // 	cout<<encrypted_summands;

  // Encrypt the data in binary representation.
  for (long i = 0; i < bitSize; ++i) {
    std::vector<long> a_vec(ea.size());
    std::vector<long> b_vec(ea.size());
    std::vector<long> c_vec(ea.size());
    // std::vector<std::vector<long>> arr1_vec(ea.size());
    // std::vector<std::vector<long>> arr2_vec(ea.size());
    // Extract the i'th bit of a,b,c.
    for (auto& slot : a_vec)
      slot = (a_data >> i) & 1;
  	  //cout<<a_vec[i];
    for (auto& slot : b_vec)
      slot = (b_data >> i) & 1;
    for (auto& slot : c_vec)
      slot = (c_data >> i) & 1;
  	// for (auto& x : arr1_vec)
  	//   for(auto& slot : x)
  	//   	slot = (arr1>>i)&1; //arr[i]?
  	// for (auto& x : arr2_vec)
  	//   for(auto& slot : x)
  	//   	slot = (arr2>>i)&1; //arr[i]?
    ea.encrypt(encrypted_a[i], public_key, a_vec);
    ea.encrypt(encrypted_b[i], public_key, b_vec);
    ea.encrypt(encrypted_c[i], public_key, c_vec);
    // ea.encrypt(encrypted_arr1[i], public_key, arr1_vec);
    // ea.encrypt(encrypted_arr2[i], public_key, arr2_vec);
  }

  // Although in general binary numbers are represented here as
  // std::vector<helib::Ctxt> the binaryArith APIs for HElib use the PtrVector
  // wrappers instead, e.g. helib::CtPtrs_vectorCt. These are nothing more than
  // thin wrapper classes to standardise access to different vector types, such
  // as NTL::Vec and std::vector. They do not take ownership of the underlying
  // object but merely provide access to it.
  //
  // helib::CtPtrMat_vectorCt is a wrapper for
  // std::vector<std::vector<helib::Ctxt>>, used for representing a list of
  // encrypted binary numbers.

  // Perform the multiplication first and put it in encrypted_product.
  std::vector<helib::Ctxt> encrypted_product;
  helib::CtPtrs_vectorCt product_wrapper(encrypted_product);
  helib::multTwoNumbers(
      product_wrapper,
      helib::CtPtrs_vectorCt(encrypted_a),
      helib::CtPtrs_vectorCt(encrypted_b),
      /*rhsTwosComplement=*/false, // This means the rhs is unsigned rather
                                   // than 2's complement.
      outSize, // Outsize is the limit on the number of bits in the output.
      &unpackSlotEncoding); // Information needed for bootstrapping.

  // Now perform the encrypted sum and put it in encrypted_result.
  std::vector<helib::Ctxt> encrypted_result;
  helib::CtPtrs_vectorCt result_wrapper(encrypted_result);
  helib::addTwoNumbers(
      result_wrapper,
      product_wrapper,
      helib::CtPtrs_vectorCt(encrypted_c),
      /*negative=*/false, // This means the number are unsigned rather than 2's
                          // complement.
      &unpackSlotEncoding); // Information needed for bootstrapping.

  // Decrypt and print the result.
  std::vector<long> decrypted_result;
  helib::decryptBinaryNums(decrypted_result, result_wrapper, secret_key, ea);
  //std::cout << "a*b+c = " << decrypted_result.back() << std::endl;

  // Now calculate the sum of a, b and c using the addManyNumbers function.
  encrypted_result.clear();
  decrypted_result.clear();
  //std::vector<std::vector<helib::Ctxt>> summands = {encrypted_a,
                                                   // encrypted_b};
  //helib::CtPtrMat_vectorCt summands_wrapper(summands);
  // helib::addManyNumbers(
  //     result_wrapper,
  //     summands_wrapper,
  //     0,                    // sizeLimit=0 means use as many bits as needed.
  //     &unpackSlotEncoding); // Information needed for bootstrapping.
  // helib::bitwiseXOR(
  // 	result_wrapper,
  //  	helib::CtPtrs_vectorCt(encrypted_a),
  //   helib::CtPtrs_vectorCt(encrypted_b)
  //   );
  std::vector<helib::Ctxt> result_vector(bitSize, helib::Ctxt(secret_key));
  helib::CtPtrs_vectorCt output_wrapper(result_vector);
  helib::subtractBinary(output_wrapper,
                    helib::CtPtrs_vectorCt(encrypted_a),
                    helib::CtPtrs_vectorCt(encrypted_b),
                    &unpackSlotEncoding);

  // Decrypt and print the result.
  helib::decryptBinaryNums(decrypted_result, output_wrapper, secret_key, ea,true);
  //std::cout << "a-b = " << decrypted_result.back() << std::endl;

  encrypted_result.clear();
  decrypted_result.clear();

  std::vector<helib::Ctxt> res_vector(bitSize, helib::Ctxt(secret_key));
  helib::CtPtrs_vectorCt output_wrap(res_vector);

  helib::bitwiseXOR(output_wrap, helib::CtPtrs_vectorCt(encrypted_a),
                    helib::CtPtrs_vectorCt(encrypted_b));

  helib::decryptBinaryNums(decrypted_result, output_wrap, secret_key, ea,true);
  std::cout << "a^b = " << decrypted_result.back() << std::endl;
  // This section calculates popcnt(a) using the fifteenOrLess4Four
  // function.
  // Note: the output i.e. encrypted_result should be of size 4
  // because 4 bits are required to represent numbers in [0,15].
  encrypted_result.resize(4lu, scratch);
  decrypted_result.clear();
  encrypted_a.pop_back(); // drop the MSB since we only support up to 15 bits.
  helib::fifteenOrLess4Four(result_wrapper,
                            helib::CtPtrs_vectorCt(encrypted_a));

  // Decrypt and print the result.
  helib::decryptBinaryNums(decrypted_result, result_wrapper, secret_key, ea);
  //std::cout << "popcnt(a) = " << decrypted_result.back() << std::endl;

  return 0;
}
