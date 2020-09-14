#pragma once
#include <GLKit/GLKMath.h>
#include <algorithm> // std::min
#include <random>
#include <simd/simd.h>
#include <span>
#include <stdlib.h> // abs
#include <vector>

namespace viz {

class Vector3 : public GLKVector3
{
public:
  Vector3(GLKVector3 v)
    : GLKVector3(v){};

  Vector3(float x, float y, float z) { *this = GLKVector3Make(x, y, z); };

  operator simd::float3() { return simd::float3{ v[0], v[1], v[2] }; }

  const float& operator[](int index) const { return v[index]; };

  bool operator<(const Vector3& other) const
  {
    if (v[0] != other[0]) {
      return v[0] < other[0];
    }
    if (v[1] != other[1]) {
      return v[1] < other[1];
    }
    return v[2] < other[2];
  }

  static Vector3 Negate(const Vector3& x) { return GLKVector3Negate(x); }

  Vector3 operator+(const Vector3& other)
  {
    return GLKVector3Add(*this, other);
  }

  Vector3 operator-(const Vector3& other)
  {
    return GLKVector3Subtract(*this, other);
  }

  Vector3 operator*(const Vector3& other)
  {
    return GLKVector3Multiply(*this, other);
  }

  Vector3 operator+(const float& value)
  {
    return GLKVector3AddScalar(*this, value);
  }

  Vector3 operator-(const float& value)
  {
    return GLKVector3SubtractScalar(*this, value);
  }

  Vector3 operator*(const float& value)
  {
    return GLKVector3MultiplyScalar(*this, value);
  }

  Vector3 operator/(const float& value)
  {
    return GLKVector3DivideScalar(*this, value);
  }

  void operator+=(const float& value)
  {
    v[0] += value;
    v[1] += value;
    v[2] += value;
  }

  void operator-=(const float& value)
  {
    v[0] -= value;
    v[1] -= value;
    v[2] -= value;
  }

  void operator*=(const float& value)
  {
    v[0] *= value;
    v[1] *= value;
    v[2] *= value;
  }

  void operator/=(const float& value)
  {
    v[0] /= value;
    v[1] /= value;
    v[2] /= value;
  }

  // /*
  //  Returns a vector whose elements are the larger of the corresponding
  //  elements of the vector arguments.
  //  */
  // GLK_INLINE GLKVector3 GLKVector3Maximum(GLKVector3 vectorLeft, GLKVector3
  // vectorRight);
  // /*
  //  Returns a vector whose elements are the smaller of the corresponding
  //  elements of the vector arguments.
  //  */
  // GLK_INLINE GLKVector3 GLKVector3Minimum(GLKVector3 vectorLeft, GLKVector3
  // vectorRight);

  // /*
  //  Returns true if all of the first vector's elements are equal to all of
  //  the second vector's arguments.
  //  */
  // GLK_INLINE bool GLKVector3AllEqualToVector3(GLKVector3 vectorLeft,
  // GLKVector3 vectorRight);
  // /*
  //  Returns true if all of the vector's elements are equal to the provided
  //  value.
  //  */
  // GLK_INLINE bool GLKVector3AllEqualToScalar(GLKVector3 vector, float
  // value);
  // /*
  //  Returns true if all of the first vector's elements are greater than all
  //  of the second vector's arguments.
  //  */
  // GLK_INLINE bool GLKVector3AllGreaterThanVector3(GLKVector3 vectorLeft,
  // GLKVector3 vectorRight);
  // /*
  //  Returns true if all of the vector's elements are greater than the
  //  provided value.
  //  */
  // GLK_INLINE bool GLKVector3AllGreaterThanScalar(GLKVector3 vector, float
  // value);
  // /*
  //  Returns true if all of the first vector's elements are greater than or
  //  equal to all of the second vector's arguments.
  //  */
  // GLK_INLINE bool GLKVector3AllGreaterThanOrEqualToVector3(GLKVector3
  // vectorLeft, GLKVector3 vectorRight);
  // /*
  //  Returns true if all of the vector's elements are greater than or equal
  //  to the provided value.
  //  */
  // GLK_INLINE bool GLKVector3AllGreaterThanOrEqualToScalar(GLKVector3
  // vector, float value);

  static Vector3 Normalize(Vector3& vector)
  {
    return GLKVector3Normalize(vector);
  };

  void Normalize() { *this = GLKVector3Normalize(*this); };

  // GLK_INLINE float GLKVector3DotProduct(GLKVector3 vectorLeft, GLKVector3
  // vectorRight); GLK_INLINE float GLKVector3Length(GLKVector3 vector);
  // GLK_INLINE float GLKVector3Distance(GLKVector3 vectorStart, GLKVector3
  // vectorEnd);

  static Vector3 lerp(Vector3& vectorStart, Vector3& vectorEnd, float t)
  {
    return GLKVector3Lerp(vectorStart, vectorEnd, t);
  }

  [[nodiscard]] Vector3 lerp(Vector3& other, float t)
  {
    return GLKVector3Lerp(*this, other, t);
  }

  // GLK_INLINE GLKVector3 GLKVector3CrossProduct(GLKVector3 vectorLeft,
  // GLKVector3 vectorRight);

  // /*
  //  Project the vector, vectorToProject, onto the vector, projectionVector.
  //  */
  // GLK_INLINE GLKVector3 GLKVector3Project(GLKVector3 vectorToProject,
  // GLKVector3 projectionVector);
};

class Vector2 : public GLKVector2
{
public:
  Vector2(GLKVector2 v)
    : GLKVector2(v){};

  Vector2(float x, float y) { *this = GLKVector2Make(x, y); };

  const float& operator[](int index) const { return v[index]; };
};

class Matrix3 : public GLKMatrix3
{
public:
  Matrix3(GLKMatrix3 m)
    : GLKMatrix3(m){};

  operator simd::float3x3()
  {
    return simd::float3x3(simd::float3{ m[0], m[1], m[2] },
                          simd::float3{ m[3], m[4], m[5] },
                          simd::float3{ m[6], m[7], m[8] });
  }
};

class Matrix4 : public GLKMatrix4
{
public:
  Matrix4(GLKMatrix4 m)
    : GLKMatrix4(m){};

  Matrix4()
    : GLKMatrix4(){};

  operator simd::float4x4()
  {
    return simd::float4x4(simd::float4{ m[0], m[1], m[2], m[3] },
                          simd::float4{ m[4], m[5], m[6], m[7] },
                          simd::float4{ m[8], m[9], m[10], m[11] },
                          simd::float4{ m[12], m[13], m[14], m[15] });
  }

  /**
   * m30, m31, and m32 correspond to the translation values tx, ty, tz,
   * respectively.
   */
  Matrix4(float m00,
          float m01,
          float m02,
          float m03,
          float m10,
          float m11,
          float m12,
          float m13,
          float m20,
          float m21,
          float m22,
          float m23,
          float m30,
          float m31,
          float m32,
          float m33)
  {
    *this = GLKMatrix4Make(m00,
                           m01,
                           m02,
                           m03,
                           m10,
                           m11,
                           m12,
                           m13,
                           m20,
                           m21,
                           m22,
                           m23,
                           m30,
                           m31,
                           m32,
                           m33);
  }

  /**
   * m[12], m[13], and m[14] correspond to the translation values tx, ty, and
   * tz, respectively.
   */
  Matrix4(float values[16]) { *this = GLKMatrix4MakeWithArray(values); }

  /**
   * The quaternion will be normalized before conversion.
   */
  Matrix4(GLKQuaternion quaternion)
  {
    *this = GLKMatrix4MakeWithQuaternion(quaternion);
  };

  static Matrix4 MakeTranslation(Vector3 v)
  {
    return GLKMatrix4MakeTranslation(v[0], v[1], v[2]);
  };

  static Matrix4 MakeTranslation(simd::float3 v)
  {
    return GLKMatrix4MakeTranslation(v[0], v[1], v[2]);
  };

  static Matrix4 MakeTranslation(float tx, float ty, float tz)
  {
    return GLKMatrix4MakeTranslation(tx, ty, tz);
  };

  static Matrix4 MakeScale(float size)
  {
    return GLKMatrix4MakeScale(size, size, size);
  }

  static Matrix4 MakeScale(float sx, float sy, float sz)
  {
    return GLKMatrix4MakeScale(sx, sy, sz);
  }

  static Matrix4 MakeRotation(float radians, float x, float y, float z)
  {
    return GLKMatrix4MakeRotation(radians, x, y, z);
  }

  static Matrix4 MakeXRotation(float radians)
  {
    return GLKMatrix4MakeXRotation(radians);
  }

  static Matrix4 MakeYRotation(float radians)
  {
    return GLKMatrix4MakeYRotation(radians);
  }

  static Matrix4 MakeZRotation(float radians)
  {
    return GLKMatrix4MakeZRotation(radians);
  }

  /**
   * Equivalent to gluPerspective.
   */
  static Matrix4 MakePerspective(float fovyRadians,
                                 float aspect,
                                 float nearZ,
                                 float farZ)
  {
    return GLKMatrix4MakePerspective(fovyRadians, aspect, nearZ, farZ);
  }

  /*
   Equivalent to glFrustum.
   */
  static Matrix4 MakeFrustum(float left,
                             float right,
                             float bottom,
                             float top,
                             float nearZ,
                             float farZ)
  {
    return GLKMatrix4MakeFrustum(left, right, bottom, top, nearZ, farZ);
  }

  /*
   Equivalent to glOrtho.
   */
  static Matrix4 MakeOrtho(float left,
                           float right,
                           float bottom,
                           float top,
                           float nearZ,
                           float farZ)
  {
    return GLKMatrix4MakeOrtho(left, right, bottom, top, nearZ, farZ);
  }

  /**
   * Equivalent to gluLookAt.
   */
  static Matrix4 MakeLookAt(float eyeX,
                            float eyeY,
                            float eyeZ,
                            float centerX,
                            float centerY,
                            float centerZ,
                            float upX,
                            float upY,
                            float upZ)
  {
    return GLKMatrix4MakeLookAt(
      eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
  }

  Matrix3 ToNormalMatrix()
  {
    bool isInvertable;
    return GLKMatrix3Transpose(
      GLKMatrix3Invert(GLKMatrix4GetMatrix3(*this), &isInvertable));
  }

  // TODO

  // /**
  //  * Returns the upper left 3x3 portion of the 4x4 matrix.
  //  */
  // GLKMatrix3 GLKMatrix4GetMatrix3(GLKMatrix4 matrix);
  // /*
  //  Returns the upper left 2x2 portion of the 4x4 matrix.
  //  */
  // GLKMatrix2 GLKMatrix4GetMatrix2(GLKMatrix4 matrix);

  // /*
  //  GLKMatrix4GetRow returns vectors for rows 0, 1, and 2 whose last component
  //  will be the translation value tx, ty, and tz, respectively. Valid row
  //  values range from 0 to 3, inclusive.
  //  */
  // GLKVector4 GLKMatrix4GetRow(GLKMatrix4 matrix, int row);
  // /*
  //  GLKMatrix4GetColumn returns a vector for column 3 whose first three
  //  components will be the translation values tx, ty, and tz. Valid column
  //  values range from 0 to 3, inclusive.
  //  */
  // GLKVector4 GLKMatrix4GetColumn(GLKMatrix4 matrix, int column);

  // /*
  //  GLKMatrix4SetRow expects that the vector for row 0, 1, and 2 will have a
  //  translation value as its last component. Valid row values range from 0 to
  //  3, inclusive.
  //  */
  // GLKMatrix4 GLKMatrix4SetRow(GLKMatrix4 matrix, int row, GLKVector4 vector);
  // /*
  //  GLKMatrix4SetColumn expects that the vector for column 3 will contain the
  //  translation values tx, ty, and tz as its first three components,
  //  respectively. Valid column values range from 0 to 3, inclusive.
  //  */
  // GLKMatrix4 GLKMatrix4SetColumn(GLKMatrix4 matrix,
  //                                int column,
  //                                GLKVector4 vector);

  // GLKMatrix4 GLKMatrix4Transpose(GLKMatrix4 matrix);

  // GLKMatrix4 GLKMatrix4Invert(GLKMatrix4 matrix, bool* __nullable
  // isInvertible); GLKMatrix4 GLKMatrix4InvertAndTranspose(GLKMatrix4 matrix,
  //                                         bool* __nullable isInvertible);

  Matrix4 operator*(const Matrix4& other)
  {
    return Matrix4(GLKMatrix4Multiply(*this, other));
  }

  // GLKMatrix4 GLKMatrix4Add(GLKMatrix4 matrixLeft, GLKMatrix4 matrixRight);
  // GLKMatrix4 GLKMatrix4Subtract(GLKMatrix4 matrixLeft, GLKMatrix4
  // matrixRight);

  // GLKMatrix4 GLKMatrix4Translate(GLKMatrix4 matrix,
  //                                float tx,
  //                                float ty,
  //                                float tz);
  // GLKMatrix4 GLKMatrix4TranslateWithVector3(GLKMatrix4 matrix,
  //                                           GLKVector3 translationVector);
  // /*
  //  The last component of the GLKVector4, translationVector, is ignored.
  //  */
  // GLKMatrix4 GLKMatrix4TranslateWithVector4(GLKMatrix4 matrix,
  //                                           GLKVector4 translationVector);

  // GLKMatrix4 GLKMatrix4Scale(GLKMatrix4 matrix, float sx, float sy, float
  // sz); GLKMatrix4 GLKMatrix4ScaleWithVector3(GLKMatrix4 matrix,
  //                                       GLKVector3 scaleVector);
  // /*
  //  The last component of the GLKVector4, scaleVector, is ignored.
  //  */
  // GLKMatrix4 GLKMatrix4ScaleWithVector4(GLKMatrix4 matrix,
  //                                       GLKVector4 scaleVector);

  // GLKMatrix4 GLKMatrix4Rotate(GLKMatrix4 matrix,
  //                             float radians,
  //                             float x,
  //                             float y,
  //                             float z);
  // GLKMatrix4 GLKMatrix4RotateWithVector3(GLKMatrix4 matrix,
  //                                        float radians,
  //                                        GLKVector3 axisVector);
  // /*
  //  The last component of the GLKVector4, axisVector, is ignored.
  //  */
  // GLKMatrix4 GLKMatrix4RotateWithVector4(GLKMatrix4 matrix,
  //                                        float radians,
  //                                        GLKVector4 axisVector);

  // GLKMatrix4 GLKMatrix4RotateX(GLKMatrix4 matrix, float radians);
  // GLKMatrix4 GLKMatrix4RotateY(GLKMatrix4 matrix, float radians);
  // GLKMatrix4 GLKMatrix4RotateZ(GLKMatrix4 matrix, float radians);

  // /*
  //  Assumes 0 in the w component.
  //  */
  // GLKVector3 GLKMatrix4MultiplyVector3(GLKMatrix4 matrixLeft,
  //                                      GLKVector3 vectorRight);
  // /*
  //  Assumes 1 in the w component.
  //  */
  // GLKVector3 GLKMatrix4MultiplyVector3WithTranslation(GLKMatrix4 matrixLeft,
  //                                                     GLKVector3
  //                                                     vectorRight);
  // /*
  //  Assumes 1 in the w component and divides the resulting vector by w before
  //  returning.
  //  */
  // GLKVector3 GLKMatrix4MultiplyAndProjectVector3(GLKMatrix4 matrixLeft,
  //                                                GLKVector3 vectorRight);

  // /*
  //  Assumes 0 in the w component.
  //  */
  // void GLKMatrix4MultiplyVector3Array(GLKMatrix4 matrix,
  //                                     GLKVector3* __nonnull vectors,
  //                                     size_t vectorCount);
  // /*
  //  Assumes 1 in the w component.
  //  */
  // void GLKMatrix4MultiplyVector3ArrayWithTranslation(GLKMatrix4 matrix,
  //                                                    GLKVector3* __nonnull
  //                                                      vectors,
  //                                                    size_t vectorCount);
  // /*
  //  Assumes 1 in the w component and divides the resulting vector by w before
  //  returning.
  //  */
  // void GLKMatrix4MultiplyAndProjectVector3Array(GLKMatrix4 matrix,
  //                                               GLKVector3* __nonnull
  //                                               vectors, size_t vectorCount);

  // GLKVector4 GLKMatrix4MultiplyVector4(GLKMatrix4 matrixLeft,
  //                                      GLKVector4 vectorRight);

  // void GLKMatrix4MultiplyVector4Array(GLKMatrix4 matrix,
  //                                     GLKVector4* __nonnull vectors,
  //                                     size_t vectorCount);
};

// ------------------------------------------------

// Taken from: https://github.com/mikolalysenko/angle-normals/pull/1
// MIT License, Mikola Lysenko, with Ricky Reusser.
template<typename Decimal, typename Integer>
std::vector<Decimal>
ComputeAngleNormalsPacked(const std::span<Integer>& cells,
                          const std::span<Decimal>& positions)
{
  size_t posSize = positions.size();
  size_t cellsSize = cells.size();

  std::vector<Decimal> normals(posSize, 0.0);

  for (auto cellIndex = 0; cellIndex < cellsSize; cellIndex += 3) {
    Integer aIdx = cells[cellIndex] * 3;
    Integer bIdx = cells[cellIndex + 1] * 3;
    Integer cIdx = cells[cellIndex + 2] * 3;

    Decimal abx = positions[bIdx] - positions[aIdx];
    Decimal aby = positions[bIdx + 1] - positions[aIdx + 1];
    Decimal abz = positions[bIdx + 2] - positions[aIdx + 2];
    Decimal ab = sqrt(abx * abx + aby * aby + abz * abz);

    Decimal bcx = positions[bIdx] - positions[cIdx];
    Decimal bcy = positions[bIdx + 1] - positions[cIdx + 1];
    Decimal bcz = positions[bIdx + 2] - positions[cIdx + 2];
    Decimal bc = sqrt(bcx * bcx + bcy * bcy + bcz * bcz);

    Decimal cax = positions[cIdx] - positions[aIdx];
    Decimal cay = positions[cIdx + 1] - positions[aIdx + 1];
    Decimal caz = positions[cIdx + 2] - positions[aIdx + 2];
    Decimal ca = sqrt(cax * cax + cay * cay + caz * caz);

    if (std::min(std::min(ab, bc), ca) < 0.000001) {
      continue;
    }

    Decimal s = 0.5 * (ab + bc + ca);
    Decimal r = sqrt((s - ab) * (s - bc) * (s - ca) / s);

    Decimal nx = aby * bcz - abz * bcy;
    Decimal ny = abz * bcx - abx * bcz;
    Decimal nz = abx * bcy - aby * bcx;
    Decimal nl = sqrt(nx * nx + ny * ny + nz * nz);
    nx /= nl;
    ny /= nl;
    nz /= nl;

    Decimal w = atan2(r, s - bc);
    normals[aIdx] += w * nx;
    normals[aIdx + 1] += w * ny;
    normals[aIdx + 2] += w * nz;

    w = atan2(r, s - ca);
    normals[bIdx] += w * nx;
    normals[bIdx + 1] += w * ny;
    normals[bIdx + 2] += w * nz;

    w = atan2(r, s - ab);
    normals[cIdx] += w * nx;
    normals[cIdx + 1] += w * ny;
    normals[cIdx + 2] += w * nz;
  }

  // Normalize all the normals
  for (size_t posIndex = 0; posIndex < posSize; posIndex += 3) {
    auto l = sqrt(normals[posIndex] * normals[posIndex] +
                  normals[posIndex + 1] * normals[posIndex + 1] +
                  normals[posIndex + 2] * normals[posIndex + 2]);

    if (l < 0.00000008) {
      normals[posIndex] = 1;
      normals[posIndex + 1] = 0;
      normals[posIndex + 2] = 0;
      continue;
    }
    normals[posIndex] /= l;
    normals[posIndex + 1] /= l;
    normals[posIndex + 2] /= l;
  }

  return normals;
}

float
Random();

float
Random(float range);

float
Random(float rangeMin, float rangeMax);

/**
 * Generates numbers that tend to be lower, based on the exponent provided.
 */
float
RandomPow(size_t pow);

float
RandomPow(float range, size_t pow);

float
RandomPow(float rangeMin, float rangeMax, size_t pow);

struct RandomSphericalInitializer
{
  float radius = 1.0;
  Vector3 center = { 0.0, 0.0, 0.0 };
};

Vector3
RandomSpherical(RandomSphericalInitializer&& initializer);

} // namespace viz
