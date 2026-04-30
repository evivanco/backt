import java.util.Scanner;

public class Calculadora {

    public static double sumar(double a, double b) {
        return a + b;
    }

    public static double restar(double a, double b) {
        return a - b;
    }

    public static double multiplicar(double a, double b) {
        return a * b;
    }

    public static double dividir(double a, double b) {
        if (b == 0) {
            throw new ArithmeticException("No se puede dividir por cero");
        }
        return a / b;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int opcion;

        do {
            System.out.println("\n--- CALCULADORA ---");
            System.out.println("1. Sumar");
            System.out.println("2. Restar");
            System.out.println("3. Multiplicar");
            System.out.println("4. Dividir");
            System.out.println("5. Salir");
            System.out.print("Seleccione una opción: ");
            
            opcion = sc.nextInt();

            if (opcion >= 1 && opcion <= 4) {
                System.out.print("Ingrese primer número: ");
                double a = sc.nextDouble();

                System.out.print("Ingrese segundo número: ");
                double b = sc.nextDouble();

                try {
                    double resultado = 0;

                    switch (opcion) {
                        case 1:
                            resultado = sumar(a, b);
                            break;
                        case 2:
                            resultado = restar(a, b);
                            break;
                        case 3:
                            resultado = multiplicar(a, b);
                            break;
                        case 4:
                            resultado = dividir(a, b);
                            break;
                    }

                    System.out.println("Resultado: " + resultado);

                } catch (ArithmeticException e) {
                    System.out.println("Error: " + e.getMessage());
                }
            }

        } while (opcion != 5);

        sc.close();
        System.out.println("Programa finalizado.");
    }
}