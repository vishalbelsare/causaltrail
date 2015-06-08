#ifndef DISCRETISER_H
#define DISCRETISER_H

#include "Discretisations.h"
#include "Network.h"
#include "float.h"
#include <map>

class Discretiser{
	public: 
	/**Discretiser
 	 *
	 * @param originalObservations, a const reference to the matrix containing the raw sample data
	 * @param obsMatrix, a reference to the new observation matrix that shall contain the discretised data
	 * @param network, a reference to the network
	 *
 	 * @return Discretiser Object
	 *
 	 *
 	 */
	Discretiser(const Matrix<std::string>& originalObservations, 
			Matrix<int>& obsMatrix, 
			Network& network);

	/**Discretiser
 	 *
	 * @param originalObservations, a const reference to the matrix containing the raw sample data
	 * @param filename, name of a "controlFile" that regulates the discretisation for each node 
	 * @param obsMatrix, a reference to the new observation matrix that shall contain the discretised data
	 * @param network, a reference to the network
	 *
 	 * @return Discretiser Object
 	 *
	 * In contrast to the before declared constructor, this constructor uses the controlFile and automatically discretises
	 * all observations that are listed in this file
 	 */
	Discretiser(const Matrix<std::string>& originalObservations,
		const std::string& filename, 
		Matrix<int>& obsMatrix, 
		Network& network);

	Discretiser& operator=(const Discretiser&) = delete;
	
	Discretiser& operator=(Discretiser&&) = delete;


	/**getEntry
 	 *
	 * @param col, colum of interest
	 * @param row, row of interest
	 *
 	 * @return Returns an integer representation for the discretised datapoint at the given position
	 *
 	 */
	int getEntry(unsigned int col, unsigned int row);


	void discretise(const std::string& controlFile);

	private:
	void createDiscretisationClasses(const std::string& controlFile);


	/**discretiseFloor
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by rounding each value to the next smaller integer value
 	 */
	void discretiseFloor(unsigned int row);

	/**discretiseZ
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by calculating the z-score for each feature and discretising accordingly.
 	 */
	void discretiseZ(unsigned int row);

	/**discretiseCeil
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by rounding each value to the next larger integer value
 	 */
	void discretiseCeil(unsigned int row);

	/**discretiseRound
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by rounding each value according to the first digit after the comma (<5, or >=5)
 	 */
	void discretiseRound(unsigned int row);

	/**discretiseByAMean
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by computing the arithmetic mean of it and comparing each value against it
 	 */
	void discretiseByAMean(unsigned int row);

	/**discretiseByHMean
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by computing the harmonic mean of it and comparing each value against it
 	 */
	void discretiseByHMean(unsigned int row);

	/**discretiseByMedian
 	 *
	 * @param row, row to discretise
	 *
 	 * Discretises the given row by computing the median of it and comparing each value against it
 	 */
	void discretiseByMedian(unsigned int row);

	/**discretiseManually
 	 *
	 * @param row, row to discretise
	 * @param threshold, a manually chossen threshold that divids the data into two groups
	 *
 	 * Discretises the given row by comparing each value against the given threshold
 	 */
	void discretiseManually(unsigned int row, float threshold);

	/**discretiseBracketMedians
 	 *
	 * @param row, row to discretise
	 * @param numberOfBrackets, number of brackets to divide the data into
	 *
 	 * Performs the BracketMedians method to discretise the data
 	 */
	void discretiseBracketMedians(unsigned int row, unsigned int numberOfBrackets);

	/**discretisePearsonTukey
 	 *
	 * @param row, row to discretise
	 *
 	 * Performs the PearsonTukey method to discretise the given row
 	 */
	void discretisePearsonTukey(unsigned int row);

	/**mapNamesToInt
 	 *
	 * @param row, row to discretise
	 *
 	 * Maps discrete values in the given row from a string to a integer representation
 	 */
	void mapNamesToInt(unsigned int row);

	/**adaptFormat
 	 *
 	 * Replaces various kinds of NA representations by a unqiue version
 	 */
	void adaptFormat();

	//Matrix containing the original raw sample data
	Matrix<std::string> originalObservations_;
	//Matrix containing the discretised data
	Matrix<int>& observations_;
	//Map from original value to integer representation
	std::unordered_map<std::string,int>& observationsMap_;
	//Map from integer representation (and observationRow from class Node) to the original representation
	std::map<std::pair<int,int>, std::string>& observationsMapR_;
	//Vector of unique pointers, pointing to discretisation objects
	std::vector<std::unique_ptr<Discretisations>> discretisations_;
};
#endif
