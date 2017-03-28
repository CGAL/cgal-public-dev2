namespace CGAL
{

namespace Classification
{

/*!
\ingroup PkgClassificationConcepts
\cgalConcept

Concept describing a predicate used by classification functions (see
`CGAL::Classification::classify()`, `CGAL::Classification::classify_with_local_smoothing()` and
`CGAL::Classification::classify_with_graphcut()`).

\cgalHasModel `CGAL::Classification::Sum_of_weighted_features_predicate`
\cgalHasModel `CGAL::Classification::Random_forest_predicate`

*/
class Predicate
{
public:

  /*!
    \brief Returns, for each label indexed from 0 to `out.size()`, the
    ernegy of this label applied to point at `item_index`.
   */
  void operator() (std::size_t item_index, std::vector<double>& out) const;


};

} // namespace Classification

} // namespace CGAL
