
namespace CGAL {

/*!
 \ingroup PkgBasicViewerClasses

The class `Graphics_scene` stores points, segments, triangles, rays and lines. Elements can be added, possibly with associated colors. Non triangular faces can be directly added and are triangulated internally.

\tparam BufferType the number type used for point coordinates: `float` by default.

*/
template <typename BufferType=float>
class Graphics_scene {
public:
  /// adds the given point in the scene.
  template <typename KPoint>
  void add_point(const KPoint &p);

  /// adds the given colored point in the scene.
  template <typename KPoint>
  void add_point(const KPoint &p, const CGAL::IO::Color &color);

  /// adds the given segment in the scene.
  template <typename KPoint>
  void add_segment(const KPoint &p1, const KPoint &p2);

  /// adds the given colored segment in the scene.
  template <typename KPoint>
  void add_segment(const KPoint &p1, const KPoint &p2,
                   const CGAL::IO::Color &color);

  /// adds the given ray in the scene: a half line starting from `p` and having `v` as direction.
  template <typename KPoint, typename KVector>
  void add_ray(const KPoint &p, const KVector &v);

  /// adds the given colored ray in the scene: a half line starting from `p` and having `v` as direction.
  template <typename KPoint, typename KVector>
  void add_ray(const KPoint &p, const KVector &v,
               const CGAL::IO::Color &color);

  /// adds the given line in the scene, defined by `p` and `v` as direction.
  template <typename KPoint, typename KVector>
  void add_line(const KPoint &p, const KVector &v);

  /// adds the given colored line in the scene, defined by `p` and `v` as direction.
  template <typename KPoint, typename KVector>
  void add_line(const KPoint &p, const KVector &v,
                const CGAL::IO::Color &color);

  /// starts a new face.
  void face_begin();

  /// starts a new colored face.
  void face_begin(const CGAL::IO::Color &color);

  /// return `true` iff a face is started.
  bool a_face_started() const;

  /// adds the given point in the current face.
  /// @pre `a_face_started()`
  template <typename KPoint> bool add_point_in_face(const KPoint &kp);

  /// adds the given point in the current face, having the vertex normal.
  /// @pre `a_face_started()`
  template <typename KPoint, typename KVector>
  bool add_point_in_face(const KPoint &kp, const KVector &p_normal);

  /// ends the current face.
  /// @pre `a_face_started()`
  void face_end();

  /// adds the given text at the given position in the scene.
  template <typename KPoint>
  void add_text(const KPoint &kp, const char *txt);

  /// adds the given text at the given position in the scene.
  template <typename KPoint>
  void add_text(const KPoint &kp, const std::string &txt);

  /// returns `true` iff the scene has no element.
  bool empty() const;

  /// clears the scene, i.e., removes all points, segments, triangles, and text.
  void clear();
};

} // namespace CGAL
