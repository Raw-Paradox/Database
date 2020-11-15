import okhttp3.FormBody;
import okhttp3.OkHttpClient;
import okhttp3.RequestBody;
import okhttp3.ResponseBody;
import retrofit2.Call;
import retrofit2.Response;
import retrofit2.Retrofit;
import retrofit2.converter.gson.GsonConverterFactory;
import retrofit2.http.Body;
import retrofit2.http.POST;

import java.io.IOException;


class Person {
    int id;
    String username;
    String email;

    public Person(int id, String username, String email) {
        this.id = id;
        this.username = username;
        this.email = email;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }
}

public class Network {
    Retrofit retrofit;
    Service service;

    Network(String url) {
        OkHttpClient client  = new OkHttpClient.Builder()
                .addInterceptor(chain -> {
                    var request = chain.request();
                    System.out.println(request.toString());
                    System.out.println(request.body());
                    return chain.proceed(request);
                })
                .build();

        retrofit = new Retrofit.Builder()
                .client(client)
                .addConverterFactory(GsonConverterFactory.create())
                .baseUrl(url)
                .build();
        service = retrofit.create(Service.class);
    }

    void insert(Person person) {
        try {
            service.insert(person).execute();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    interface Service {
        @POST("/insert")
        Call<ResponseBody> insert(@Body Person person);
    }
}
