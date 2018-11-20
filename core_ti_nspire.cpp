
#include <nspireio/console.hpp>
#include <inttypes.h>
#include <math.h>
#include <cstdlib>

#ifndef TRUE
#define TRUE 1
#endif // TRUE

#ifndef FALSE
#define FALSE 0
#endif // FALSE

nio::console csl;

constexpr uint8_t NETLIST_CONNECTION_MAX = 4;
constexpr uint8_t NETLIST_PARAM_MAX = 5;
constexpr uint8_t NETLIST_ALIAS_SIZE = 16;
constexpr double MATH_INVERSE_TOL = 10e-15;
double GMIN;
//NODE_REF
//2 port
constexpr uint8_t positive = 0;
constexpr uint8_t negative = 1;
//voltage_source trick
constexpr uint8_t hidden = 2;
//mosfet
constexpr uint8_t fet_source = 0;
constexpr uint8_t fet_drain = 1;
constexpr uint8_t fet_gate = 2;

//VALUE_REF
//voltage_source
constexpr uint8_t voltage = 0;
//resistor
constexpr uint8_t resistance = 0;
//mosfet
constexpr uint8_t fet_w = 0;
constexpr uint8_t fet_l = 1;
constexpr uint8_t fet_kp = 2;
constexpr uint8_t fet_vt = 3;
constexpr uint8_t fet_lambda = 4;

//TYPE_REF
constexpr uint16_t resistor = 0;
constexpr uint16_t voltage_source = 1;
constexpr uint16_t current_source = 2;
constexpr uint16_t capacitor = 3;
constexpr uint16_t inductor = 4;
constexpr uint16_t diode = 5;
constexpr uint16_t bjt_npn = 6;
constexpr uint16_t bjt_pnp = 7;
constexpr uint16_t fet_n = 8;
constexpr uint16_t fet_p = 9;

class utils_t {
public:
	template <typename T> static void copy_vect(T *, T*, uint16_t);
	template <typename T> static void copy_matrix(T**, T**, uint16_t, uint16_t);
	template <typename T> static void zero_vect(T *, uint16_t);
	template <typename T> static void zero_matrix(T**, uint16_t, uint16_t);
	template <typename T> static T mult_matrix_single_row_col(T**, T**, uint16_t, uint16_t, uint16_t);
	template <typename T> static void mult_mat_mat(T**, T**, T**, uint16_t, uint16_t, uint16_t);
	template <typename T> static void mult_mat_vect(T**, T*, T*, uint16_t, uint16_t);
	template <typename T> static void mult_vect_num(T*, T, T*, uint16_t);
	template <typename T> static double norm(T*, uint16_t);
	static char* double_to_ascii(double, char*);
};
class netlist_t {
public:
    struct row_t;
	uint8_t nodes;
	uint8_t components;
	row_t *row;
	netlist_t(uint8_t nod, uint8_t comp);
	~netlist_t(void);
};
class component_t {
public:
	static void insert_all(double **G, double *X, netlist_t *netlist, uint8_t netlist_pos, uint8_t *vs_pos);
	static void nop_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target);
	static void voltage_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target);
    static void fet_n_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target);
    static void fet_p_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target);
};
class nod_t{
public:
    uint8_t nodes;//how many nodes
	uint8_t dim;//nodal arrays length
	uint8_t components;//netlist size
	netlist_t *netlist;//source of a lot of data
	double LFS0;
	double *X;
	double *X_temp;
    double *S0;
    double *FS0;
    double *S0_temp;
    double *S1;
    double *FS1;
    double *S1_best;
    double *FS1_best;
    double *T1;
    double *NT1;
    double *U1;
    double *update_array_temp;
    double **G;//conductance
    double **R;//resistance
    double **JN;//Jacobian
    double **JNI;//inverse Jacobian
    double **updated_jacobian(double);//finds the numerical Jacobian
    void update_array(double *, double *);
    double eval_row(uint8_t, double *, double *);
    double* eval_f(double *, double *, double *);
	uint32_t iterate(uint32_t, double, double, double);
    nod_t(netlist_t *source);
    ~nod_t(void);
};
class math_t {
public:
	static void sub(double *, double *, double *, uint8_t);
	static void add(double *, double *, double *, uint8_t);
	static double** invert(double **, double **, uint8_t);
private:
	static int LUPDecompose(double **A, int N, double Tol, int *P);
	static void LUPSolve(double **A, int *P, double *b, int N, double *x);
	static void LUPInvert(double **A, int *P, int N, double **IA);
	static double LUPDeterminant(double **A, int *P, int N);
};

template <typename T> void utils_t::copy_vect(T *from, T *to, uint16_t length) {
	while (length) {
		length--;
		to[length] = from[length];
	}
}
template <typename T> void utils_t::copy_matrix(T** from, T** to, uint16_t i, uint16_t j) {
	while (i) {
		i--;
		copy_vect(from[i], to[i], j);
	}
}
template <typename T> void utils_t::zero_vect(T *vect, uint16_t length) {
	while (length) {
		length--;
		vect[length] = 0;
	}
}
template <typename T> void utils_t::zero_matrix(T** M, uint16_t i, uint16_t j) {
	while (i) {
		i--;
		zero_vect(M[i], j);
	}
}
template <typename T> T utils_t::mult_matrix_single_row_col(T** M, T** N, uint16_t i, uint16_t j, uint16_t len) {
	T acc = 0;
	while (len) {
		len--;
		acc += M[i][len] * N[len][j];
	}
	return acc;
}
template <typename T> void utils_t::mult_mat_mat(T** M, T** N, T** OUT, uint16_t i1, uint16_t j2, uint16_t len) {
	uint16_t x;
	while (i1){
		i1--;
		x = j2;
		while (x){
			x--;
			OUT[i1][x] = mult_matrix_single_row_col(M, N, i1, x, len);
		}
	}
}
template <typename T> void utils_t::mult_mat_vect(T** M, T* N, T* OUT, uint16_t i_max, uint16_t j_max) {
	uint16_t j;
	while (i_max){
		i_max--;
		OUT[i_max] = (T)0;
		j = j_max;
		while (j){
			j--;
			OUT[i_max] += M[i_max][j] * N[j];
		}
	}
}
template <typename T> void utils_t::mult_vect_num(T* vect, T num, T* out, uint16_t len){
	while (len){
		len--;
		out[len] = vect[len]*num;
	}
}

template <typename T> double utils_t::norm(T* vect, uint16_t len){
    double acc = 0;
    while(len){
        len--;
        acc+=pow(vect[len],2);
    }
    return sqrt(acc);
}
char* utils_t::double_to_ascii(double number, char *str) {
    //copied from androider @ stackoverflow
    // handle special cases
    const double PRECISION = 0.00000000000001;
    const int MAX_NUMBER_STRING_SIZE = 32;
    double n = number;
    char *s = str;
    if (isnan(n)) {
        strcpy(s, "nan");
    } else if (isinf(n)) {
        strcpy(s, "inf");
    } else if (n == 0.0) {
        strcpy(s, "0");
    } else {
        int digit, m, m1;
        char *c = s;
        int neg = (n < 0);
        if (neg)
            n = -n;
        // calculate magnitude
        m = log10(n);
        int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
        if (neg)
            *(c++) = '-';
        // set up for scientific notation
        if (useExp) {
            if (m < 0)
               m -= 1.0;
            n = n / pow(10.0, m);
            m1 = m;
            m = 0;
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0)
                *(c++) = '.';
            m--;
        }
        if (useExp) {
            // convert the exponent
            int i, j;
            *(c++) = 'e';
            if (m1 > 0) {
                *(c++) = '+';
            } else {
                *(c++) = '-';
                m1 = -m1;
            }
            m = 0;
            while (m1 > 0) {
                *(c++) = '0' + m1 % 10;
                m1 /= 10;
                m++;
            }
            c -= m;
            for (i = 0, j = m-1; i<j; i++, j--) {
                // swap without temporary
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += m;
        }
        *(c) = '\0';
    }
    return s;
}

struct netlist_t::row_t{
    uint8_t type;
    double value[NETLIST_PARAM_MAX];
    uint8_t node[NETLIST_CONNECTION_MAX];
    void (*update)(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target);
    char alias[NETLIST_ALIAS_SIZE];
};

netlist_t::netlist_t(uint8_t amnt_nodes, uint8_t amnt_comp) {
	nodes = amnt_nodes - 1;
	components = amnt_comp;
	if (amnt_nodes) {
		row = new row_t[amnt_comp];
	}
}
netlist_t::~netlist_t() {
	if (nodes) {
		delete[] row;
	}
}


void component_t::insert_all(double **G, double *X, netlist_t *netlist, uint8_t netlist_pos, uint8_t *vs_pos) {
    while(netlist_pos){
        netlist_pos--;
        switch (netlist->row[netlist_pos].type){
        case(voltage_source):
            if(netlist->row[netlist_pos].node[positive]){
                G[netlist->row[netlist_pos].node[positive] - 1][*vs_pos - 1] = 1;
                G[*vs_pos - 1][netlist->row[netlist_pos].node[positive] - 1] = 1;
            }
            if(netlist->row[netlist_pos].node[negative]){
                G[netlist->row[netlist_pos].node[positive] - 1][*vs_pos - 1] = -1;
                G[*vs_pos - 1][netlist->row[netlist_pos].node[positive] - 1] = -1;
            }
            netlist->row[netlist_pos].node[hidden] = *vs_pos;
            netlist->row[netlist_pos].update = voltage_f;
            (*vs_pos)--;
            break;
        case(current_source):
            //remember to += memory (current)
            break;
        case(resistor):
            if (netlist->row[netlist_pos].node[positive]) {
                G[netlist->row[netlist_pos].node[positive] - 1][netlist->row[netlist_pos].node[positive] - 1] += 1 / netlist->row[netlist_pos].value[resistance];
                if (netlist->row[netlist_pos].node[negative]) {
                    G[netlist->row[netlist_pos].node[positive] - 1][netlist->row[netlist_pos].node[negative] - 1] += -1 / netlist->row[netlist_pos].value[resistance];
                }
            }
            if (netlist->row[netlist_pos].node[negative]) {
                G[netlist->row[netlist_pos].node[negative] - 1][netlist->row[netlist_pos].node[negative] - 1] += 1 / netlist->row[netlist_pos].value[resistance];
                if (netlist->row[netlist_pos].node[positive]) {
                    G[netlist->row[netlist_pos].node[negative] - 1][netlist->row[netlist_pos].node[positive] - 1] += -1 / netlist->row[netlist_pos].value[resistance];
                }
            }
            netlist->row[netlist_pos].update = nop_f;
            break;
        case(capacitor):

            break;
        case(inductor):

            break;
        case(diode):
            if (netlist->row[netlist_pos].node[positive]) {
                G[netlist->row[netlist_pos].node[positive] - 1][netlist->row[netlist_pos].node[positive] - 1] += GMIN;
                if (netlist->row[netlist_pos].node[negative]) {
                    G[netlist->row[netlist_pos].node[positive] - 1][netlist->row[netlist_pos].node[negative] - 1] += -GMIN;
                }
            }
            if (netlist->row[netlist_pos].node[negative]) {
                G[netlist->row[netlist_pos].node[negative] - 1][netlist->row[netlist_pos].node[negative] - 1] += GMIN;
                if (netlist->row[netlist_pos].node[positive]) {
                    G[netlist->row[netlist_pos].node[negative] - 1][netlist->row[netlist_pos].node[positive] - 1] += -GMIN;
                }
            }
            break;
        case (fet_n):
            if (netlist->row[netlist_pos].node[fet_source]) {
                G[netlist->row[netlist_pos].node[fet_source] - 1][netlist->row[netlist_pos].node[fet_source] - 1] += GMIN;
                if (netlist->row[netlist_pos].node[fet_drain]) {
                    G[netlist->row[netlist_pos].node[fet_source] - 1][netlist->row[netlist_pos].node[fet_drain] - 1] += -GMIN;
                }
            }
            if (netlist->row[netlist_pos].node[fet_drain]) {
                G[netlist->row[netlist_pos].node[fet_drain] - 1][netlist->row[netlist_pos].node[fet_drain] - 1] += GMIN;
                if (netlist->row[netlist_pos].node[fet_source]) {
                    G[netlist->row[netlist_pos].node[fet_drain] - 1][netlist->row[netlist_pos].node[fet_source] - 1] += -GMIN;
                }
            }
            netlist->row[netlist_pos].update = fet_n_f;
            break;
        case(fet_p):
            if (netlist->row[netlist_pos].node[fet_source]) {
                G[netlist->row[netlist_pos].node[fet_source] - 1][netlist->row[netlist_pos].node[fet_source] - 1] += GMIN;
                if (netlist->row[netlist_pos].node[fet_drain]) {
                    G[netlist->row[netlist_pos].node[fet_source] - 1][netlist->row[netlist_pos].node[fet_drain] - 1] += -GMIN;
                }
            }
            if (netlist->row[netlist_pos].node[fet_drain]) {
                G[netlist->row[netlist_pos].node[fet_drain] - 1][netlist->row[netlist_pos].node[fet_drain] - 1] += GMIN;
                if (netlist->row[netlist_pos].node[fet_source]) {
                    G[netlist->row[netlist_pos].node[fet_drain] - 1][netlist->row[netlist_pos].node[fet_source] - 1] += -GMIN;
                }
            }
            netlist->row[netlist_pos].update = fet_p_f;
            break;
        default:
            break;
        }
    }
}

void component_t::nop_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target){
}
void component_t::voltage_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target){
    target[netlist->row[netlist_pos].node[hidden] - 1] = netlist->row[netlist_pos].value[voltage];
}
void component_t::fet_n_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target){
    double vs, vd, vg, vds, vgs, id, w, l, kp, vt, lambda;
    w = netlist->row[netlist_pos].value[fet_w];
    l = netlist->row[netlist_pos].value[fet_l];
    kp = netlist->row[netlist_pos].value[fet_kp];
    vt = netlist->row[netlist_pos].value[fet_vt];
    lambda = netlist->row[netlist_pos].value[fet_lambda];
    if(netlist->row[netlist_pos].node[fet_source]){
        vs = data[netlist->row[netlist_pos].node[fet_source] - 1];
    }
    else{
        vs = 0;
    }
    if(netlist->row[netlist_pos].node[fet_drain]){
        vd = data[netlist->row[netlist_pos].node[fet_drain] - 1];
    }
    else{
        vd = 0;
    }
    if(netlist->row[netlist_pos].node[fet_gate]){
        vg = data[netlist->row[netlist_pos].node[fet_gate] - 1];
    }
    else{
        vg = 0;
    }
    vgs = vg-vs;
    vds = vd-vs;
    if(vgs <= vt){
        id = 0;
    }
    else if(vd<(vg-vt)){
        id = (kp/2)*(w/l)*(2*(vgs-vt)*(vds)-pow(vds, 2));
    }
    else{
        id = (kp/2)*(w/l)*pow(vgs-vt, 2)*(1+lambda*vds);
    }
    if(netlist->row[netlist_pos].node[fet_source]){
        target[netlist->row[netlist_pos].node[fet_source] - 1] += id;
    }
    if(netlist->row[netlist_pos].node[fet_drain]){
        target[netlist->row[netlist_pos].node[fet_drain] - 1] -= id;
    }
}
void component_t::fet_p_f(uint8_t netlist_pos, netlist_t* netlist, double *data, double *target){
    double vs, vd, vg, vds, vgs, id, w, l, kp, vt, lambda;
    w = netlist->row[netlist_pos].value[fet_w];
    l = netlist->row[netlist_pos].value[fet_l];
    kp = netlist->row[netlist_pos].value[fet_kp];
    vt = netlist->row[netlist_pos].value[fet_vt];
    lambda = netlist->row[netlist_pos].value[fet_lambda];
    if(netlist->row[netlist_pos].node[fet_source]){
        vs = data[netlist->row[netlist_pos].node[fet_source] - 1];
    }
    else{
        vs = 0;
    }
    if(netlist->row[netlist_pos].node[fet_drain]){
        vd = data[netlist->row[netlist_pos].node[fet_drain] - 1];
    }
    else{
        vd = 0;
    }
    if(netlist->row[netlist_pos].node[fet_gate]){
        vg = data[netlist->row[netlist_pos].node[fet_gate] - 1];
    }
    else{
        vg = 0;
    }
    vgs = vg-vs;
    vds = vd-vs;
    if(vgs >= vt){
        id = 0;
    }
    else if(vd>(vg-vt)){
        id = (kp/2)*(w/l)*(2*(vgs-vt)*(vds)-pow(vds, 2));
    }
    else{
        id = (kp/2)*(w/l)*pow(vgs-vt, 2)*(1+lambda*vds);
    }
    if(netlist->row[netlist_pos].node[fet_source]){
        target[netlist->row[netlist_pos].node[fet_source] - 1] -= id;
    }
    if(netlist->row[netlist_pos].node[fet_drain]){
        target[netlist->row[netlist_pos].node[fet_drain] - 1] += id;
    }
}

nod_t::nod_t(netlist_t *src) {
	uint8_t pos = src->components;
	uint8_t voltages;
	netlist = src;
	nodes = src->nodes;
	dim = src->nodes;
	components = src->components;
	if (dim) {
		while (pos) {
			pos--;
            switch (src->row[pos].type){
			case (voltage_source):
				dim++;
				break;
			default:
				break;
			}
		}
		X = new double[dim];
		X_temp = new double[dim];
        S0 = new double[dim];
        FS0 = new double[dim];
        S0_temp = new double[dim];
        S1 = new double[dim];
        FS1 = new double[dim];
        S1_best = new double[dim];
        FS1_best = new double[dim];
        T1 = new double[dim];
        NT1 = new double[dim];
        U1 = new double[dim];
        update_array_temp = new double[dim];
        G = new double*[dim];
        R = new double*[dim];
        JN = new double*[dim];
        JNI = new double*[dim];
		pos = dim;
		while (pos) {
			pos--;
			G[pos] = new double[dim];
		}
		pos = dim;
		while (pos) {
			pos--;
			R[pos] = new double[dim];
		}
		pos = dim;
		while (pos) {
			pos--;
			JN[pos] = new double[dim];
		}
		pos = dim;
		while (pos) {
			pos--;
			JNI[pos] = new double[dim];
		}
		utils_t::zero_vect(X, dim);
		utils_t::zero_vect(S0, dim);
		utils_t::zero_matrix(G, dim, dim);
		utils_t::zero_matrix(R, dim, dim);
		//utils_t::zero_matrix(JNI, dim, dim);
		voltages = dim;
		component_t::insert_all(G, X, netlist, components, &voltages);
		math_t::invert(G, R, dim);
	}
}
nod_t::~nod_t() {
    delete[] X;
	delete[] X_temp;
    delete[] S0;
    delete[] FS0;
    delete[] S0_temp;
    delete[] S1;
    delete[] FS1;
    delete[] S1_best;
    delete[] FS1_best;
    delete[] T1;
    delete[] NT1;
    delete[] U1;
    delete[] update_array_temp;
	uint8_t pos = dim;
	if (dim) {
		while (pos) {
			pos--;
			delete[] G[pos];
		}
		delete[] G;
		pos = dim;
		while (pos) {
			pos--;
			delete[] R[pos];
		}
		delete[] R;
		pos = dim;
		while (pos) {
			pos--;
			delete[] JN[pos];
		}
		delete[] JN;
		pos = dim;
		while (pos) {
			pos--;
			delete[] JNI[pos];
		}
		delete[] JNI;
	}
}

double** nod_t::updated_jacobian(double jacobian_step) {
	uint8_t i = dim;
	uint8_t j;
	utils_t::copy_vect(S0, S0_temp, dim);
	while (i) {
		i--;
		j = dim;
		while (j) {
			j--;
			S0_temp[j] += jacobian_step;
			update_array(S0, X);
			update_array(S0_temp, X_temp);
			JN[i][j] = (eval_row(i, S0_temp, X_temp) - eval_row(i, S0, X)) / jacobian_step;
			S0_temp[j] -= jacobian_step;
		}
	}
	return JN;
}
void nod_t::update_array(double *data, double *target){
    uint8_t pos = components;
    utils_t::zero_vect(target, dim);
    while(pos){
        pos--;
        netlist->row[pos].update(pos, netlist, data, target);
    }
}

double nod_t::eval_row(uint8_t i, double *yy, double *xx) {
	double acc;
	uint8_t aux = dim;
	acc = yy[i];
	while (aux) {
		aux--;
		acc -= R[i][aux] * xx[aux];
	}
	return acc;
}
double* nod_t::eval_f(double *yy, double *xx, double *output) {
	uint8_t i_max = dim;
	update_array(yy, xx);
	while (i_max){
		i_max--;
		output[i_max] = eval_row(i_max, yy, xx);
	}
	return output;
}
uint32_t nod_t::iterate(uint32_t k, double sub_step, double error_max, double jacobian_step) {
    uint8_t unlimited = !k;
    uint8_t not_done;
    uint8_t first_sub_step;
    double LFS1_best;
    double LT1;
    double LFS1;
    double lambda;
    eval_f(S0, X, FS0);
    LFS0 = utils_t::norm(FS0, dim);
    while((LFS0 > error_max) && (k || unlimited)){
        if(k){
            k--;
        }
        lambda = 1;
        not_done = TRUE;
        //while (not_done){
            /* T1 = Jn^-1*F(S0);
             * S1 = S0 - Lam * T1;
             * if(len(F(S1)) > len(F(S0))){
             *     if(Lam*len(T1) > STEP){
             *         Lam /= 10;
             *     }
             *     else{
             *         S0 -= STEP*T1/len(T1);
             *         not_done = FALSE;
             *     }
             * }
             * else{
             *     S0 = S1;
             *     not_done = FALSE;
             * }
             */

            /* "static double FS0"
             * "static double LFS0"
             * JNI = Jn^-1;
             * T1 = JNI*FS0;
             * LT1 = len(T1);
             * NT1 = T1/LT1;
             * U1 = Lam*T1;
             * S1 = S0 - U1;
             * FS1 = F(S1);
             * LFS1 = len(FS1);
             * if(LFS1 > LFS0){
             *     if(U1 > STEP){
             *         Lam /= 10;
             *     }
             *     else{
             *         S0 -= STEP*NT1;
             *         not_done = FALSE;
             *     }
             * }
             * else{
             *     S0 = S1;
             *     FS0  = FS1;
             *     LFS0 = LFS1;
             *     not_done = FALSE;
             * }
             */

            /* "static double FS0"
             * "static double LFS0"
             * "FIRST = TRUE"
             * "BEST_S1";
             * "BEST_LFS1";
             * JNI = Jn^-1;  //<<<BEFORE WHILE
             * T1 = JNI*FS0; //<<<BEFORE WHILE
             * LT1 = len(T1);
             * NT1 = T1/LT1;
             * U1 = Lam*T1;
             * S1 = S0 - U1;
             * FS1 = F(S1);
             * LFS1 = len(FS1);
             * if(LFS1 >= LFS0){
             *     if(U1 >= STEP){
             *         Lam /= 10;
             *         if((BEST_LFS1 > LFS1) || FIRST){
             *             FIRST = FALSE;
             *             BEST_S1 = S1;
             *             BEST_LFS1 = LFS1;
             *         }
             *     }
             *     else{
             *         S0 = BEST_S1;
             *         not_done = FALSE;
             *     }
             * }
             * else{
             *     S0 = S1;
             *     FS0  = FS1;
             *     LFS0 = LFS1;
             *     not_done = FALSE;
             * }
             */
        //}
        first_sub_step = TRUE;
        math_t::invert(updated_jacobian(jacobian_step), JNI, dim);
        utils_t::mult_mat_vect(JNI, FS0, T1, dim, dim);
        LT1 = utils_t::norm(T1, dim);
        utils_t::mult_vect_num(T1, 1/LT1, NT1, dim);
        while(not_done){
            utils_t::mult_vect_num(T1, lambda, U1, dim);
            math_t::sub(S0, U1, S1, dim);
            eval_f(S1, X, FS1);
            LFS1 = utils_t::norm(FS1, dim);
            if(LFS1 > LFS0){
                if(utils_t::norm(U1, dim) >= sub_step){
                    lambda /= 10;
                    if((LFS1_best > LFS1) || first_sub_step){
                        first_sub_step = FALSE;
                        utils_t::copy_vect(S1, S1_best, dim);
                        utils_t::copy_vect(FS1, FS1_best, dim);
                        LFS1_best = LFS1;
                    }
                }
                else{
                    not_done = FALSE;
                    if(first_sub_step){
                        utils_t::copy_vect(S1, S0, dim);
                        utils_t::copy_vect(FS1, FS0, dim);
                        LFS0 = LFS1;
                    }
                    else{
                        utils_t::copy_vect(S1_best, S0, dim);
                        utils_t::copy_vect(FS1_best, FS0, dim);
                        LFS0 = LFS1_best;
                    }
                }
            }
            else{
                not_done = FALSE;
                utils_t::copy_vect(S1, S0, dim);
                utils_t::copy_vect(FS1, FS0, dim);
                LFS0 = LFS1;
            }
        }
    }
    return k;
}



void math_t::sub(double *a, double *b, double *out, uint8_t length) {
	while (length) {
		length--;
		out[length] = a[length] - b[length];
	}
}
void math_t::add(double *a, double *b, double *out, uint8_t length) {
	while (length) {
		length--;
		out[length] = a[length] + b[length];
	}
}
int math_t::LUPDecompose(double **A, int N, double Tol, int *P) {

	int i, j, k, imax;
	double maxA, *ptr, absA;

	for (i = 0; i <= N; i++)
		P[i] = i; //Unit permutation matrix, P[N] initialized with N

	for (i = 0; i < N; i++) {
		maxA = 0.0;
		imax = i;

		for (k = i; k < N; k++)
			if ((absA = fabs(A[k][i])) > maxA) {
				maxA = absA;
				imax = k;
			}

		if (maxA < Tol) return 0; //failure, matrix is degenerate

		if (imax != i) {
			//pivoting P
			j = P[i];
			P[i] = P[imax];
			P[imax] = j;

			//pivoting rows of A
			ptr = A[i];
			A[i] = A[imax];
			A[imax] = ptr;

			//counting pivots starting from N (for determinant)
			P[N]++;
		}

		for (j = i + 1; j < N; j++) {
			A[j][i] /= A[i][i];

			for (k = i + 1; k < N; k++)
				A[j][k] -= A[j][i] * A[i][k];
		}
	}

	return 1;  //decomposition done
}
void math_t::LUPSolve(double **A, int *P, double *b, int N, double *x) {

	for (int i = 0; i < N; i++) {
		x[i] = b[P[i]];

		for (int k = 0; k < i; k++)
			x[i] -= A[i][k] * x[k];
	}

	for (int i = N - 1; i >= 0; i--) {
		for (int k = i + 1; k < N; k++)
			x[i] -= A[i][k] * x[k];

		x[i] = x[i] / A[i][i];
	}
}
void math_t::LUPInvert(double **A, int *P, int N, double **IA) {

	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {
			if (P[i] == j)
				IA[i][j] = 1.0;
			else
				IA[i][j] = 0.0;

			for (int k = 0; k < i; k++)
				IA[i][j] -= A[i][k] * IA[k][j];
		}

		for (int i = N - 1; i >= 0; i--) {
			for (int k = i + 1; k < N; k++)
				IA[i][j] -= A[i][k] * IA[k][j];

			IA[i][j] = IA[i][j] / A[i][i];
		}
	}
}
double math_t::LUPDeterminant(double **A, int *P, int N) {

	double det = A[0][0];

	for (int i = 1; i < N; i++)
		det *= A[i][i];

	if ((P[N] - N) % 2 == 0)
		return det;
	else
		return -det;
}
double** math_t::invert(double **in, double** out, uint8_t dim) {
	int *P = new int[dim];
	double ** A;
	A = new double*[dim];
	uint8_t pos = dim;
	while (pos) {
		pos--;
		A[pos] = new double[dim];
	}
	utils_t::copy_matrix(in, A, dim, dim);
	math_t::LUPDecompose(A, dim, MATH_INVERSE_TOL, P);
	math_t::LUPInvert(A, P, dim, out);
	pos = dim;
	while (pos) {
		pos--;
		delete[] A[pos];
	}
	delete[] A;
	return out;
}

class spice_t{
public:
    uint32_t iterations;
    spice_t();
    ~spice_t();
    netlist_t *netlist;
    nod_t *solver;
    void simulate(void);
    void request_components();
    void show_result(void);
    void show_voltages(void);
    void show_statistics(void);
};
spice_t::spice_t(){
    int nodes;
    int components;
    int gmin;
    csl << "Nodes: ";
    csl >> nodes;
    csl << "Components: ";
    csl >> components;
    csl << "GMIN (0 = default): ";
    csl >> gmin;
    GMIN = gmin ? gmin : 10e-12;
    netlist = new netlist_t(nodes, components);
    request_components();
    solver = new nod_t(netlist);
}
spice_t::~spice_t(){
    delete netlist;
}
void spice_t::simulate(){
    double step;
    double close_enough;
    double jacobian_step;
    char strbuff[64];
    csl << "Iterations (0 = unlimited): ";
    csl >> strbuff;
    iterations = atof(strbuff);
    csl << "Bad step max length [V] (0 = default): ";
    csl >> strbuff;
    step = atof(strbuff);
    step = step ? step : 1e-3;
    csl << "Error vector max length [V] (0 = default): ";
    csl >> strbuff;
    close_enough = atof(strbuff);
    close_enough = close_enough ? close_enough : 1e-6;
    csl << "Numeric derivative step [V] (0 = default): ";
    csl >> strbuff;
    jacobian_step = atof(strbuff);
    jacobian_step = jacobian_step ? jacobian_step : 1e-10;
    iterations -= solver->iterate(iterations, step, close_enough, jacobian_step);
}
void spice_t::request_components(){
    uint16_t type;
    uint8_t pos = netlist->components;
    int gambiarra;
    char strbuff[64];
    while(pos){
        pos--;
        csl << "Component: ";
        csl >> gambiarra;
        type = gambiarra;
        csl << "Alias: ";
        csl >> netlist->row[pos].alias;
        switch(type){
        case(resistor):
            netlist->row[pos].type = type;
            csl << "Node +: ";
            csl >> gambiarra;
            netlist->row[pos].node[positive] = gambiarra;
            csl << "Node -: ";
            csl >> gambiarra;
            netlist->row[pos].node[negative] = gambiarra;
            csl << "Resistance: ";
            csl >> strbuff;
            netlist->row[pos].value[resistance] = atof(strbuff);
            break;
        case(voltage_source):
            netlist->row[pos].type = type;
            csl << "Node +: ";
            csl >> gambiarra;
            netlist->row[pos].node[positive] = gambiarra;
            csl << "Node -: ";
            csl >> gambiarra;
            netlist->row[pos].node[negative] = gambiarra;
            csl << "Voltage: ";
            csl >> strbuff;
            netlist->row[pos].value[voltage] = atof(strbuff);
            break;
        case(fet_n):
            netlist->row[pos].type = type;
            csl << "Drain node: ";
            csl >> gambiarra;
            netlist->row[pos].node[fet_drain] = gambiarra;
            csl << "Source node: ";
            csl >> gambiarra;
            netlist->row[pos].node[fet_source] = gambiarra;
            csl << "Gate node: ";
            csl >> gambiarra;
            netlist->row[pos].node[fet_gate] = gambiarra;
            csl << "W: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_w] = atof(strbuff);
            csl << "L: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_l] = atof(strbuff);
            csl << "Kp: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_kp] = atof(strbuff);
            csl << "Lambda: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_lambda] = atof(strbuff);
            csl << "Vth: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_vt] = atof(strbuff);
            break;
        case(fet_p):
            netlist->row[pos].type = type;
            csl << "Drain node: ";
            csl >> gambiarra;
            netlist->row[pos].node[fet_drain] = gambiarra;
            csl << "Source node: ";
            csl >> gambiarra;
            netlist->row[pos].node[fet_source] = gambiarra;
            csl << "Gate node: ";
            csl >> gambiarra;
            netlist->row[pos].node[fet_gate] = gambiarra;
            csl << "W: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_w] = atof(strbuff);
            csl << "L: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_l] = atof(strbuff);
            csl << "Kp: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_kp] = atof(strbuff);
            csl << "Lambda: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_lambda] = atof(strbuff);
            csl << "Vth: ";
            csl >> strbuff;
            netlist->row[pos].value[fet_vt] = atof(strbuff);
            break;
        }
    }
}
void spice_t::show_result(){
    int pos = netlist->nodes;
    char tempstr[32];
    while (pos){
        pos--;
        csl << "Node " << pos+1 << ": " << utils_t::double_to_ascii(solver->S0[pos], tempstr) << "V" << nio::endl;
    }
}
void spice_t::show_voltages(){
    uint16_t pos = netlist->components;
    char tempstr[32];
    while(pos){
        pos--;
        switch(netlist->row[pos].type){
        case(fet_n):
            csl << netlist->row[pos].alias << " - Vds: " << utils_t::double_to_ascii(solver->S0[netlist->row[pos].node[fet_drain] - 1] - solver->S0[netlist->row[pos].node[fet_source] - 1], tempstr) << "V" <<nio::endl;
            csl << netlist->row[pos].alias << " - Vgs: " << utils_t::double_to_ascii(solver->S0[netlist->row[pos].node[fet_gate] - 1] - solver->S0[netlist->row[pos].node[fet_source] - 1], tempstr) << "V" <<nio::endl;
            break;
        case(fet_p):
            csl << netlist->row[pos].alias << " - Vds: " << utils_t::double_to_ascii(solver->S0[netlist->row[pos].node[fet_drain] - 1] - solver->S0[netlist->row[pos].node[fet_source] - 1], tempstr) << "V" <<nio::endl;
            csl << netlist->row[pos].alias << " - Vgs: " << utils_t::double_to_ascii(solver->S0[netlist->row[pos].node[fet_gate] - 1] - solver->S0[netlist->row[pos].node[fet_source] - 1], tempstr) << "V" <<nio::endl;
            break;
        default:
            csl << netlist->row[pos].alias << ": " << utils_t::double_to_ascii(solver->S0[netlist->row[pos].node[positive] - 1] - solver->S0[netlist->row[pos].node[negative] - 1], tempstr) << "V" <<nio::endl;
            break;
        }
    }
}
void spice_t::show_statistics(){
    char tempstr[32];
    csl << "LFS0: " << utils_t::double_to_ascii(solver->LFS0, tempstr) << nio::endl;
    csl << "Iterations: " << (int)iterations << nio::endl;
}

int main() {
	spice_t spice;
	spice.simulate();
	csl << "-----" << nio::endl;
	spice.show_statistics();
	csl << "-----" << nio::endl;
    spice.show_result();
    csl << "-----" << nio::endl;
    spice.show_voltages();
    wait_key_pressed();
	return 0;
}

